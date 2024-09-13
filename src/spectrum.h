#pragma once

#include "platform/m256.h"
#include "platform/concurrency.h"
#include "platform/file_io.h"
#include "platform/time_stamp_counter.h"

#include "network_messages/entity.h"

#include "public_settings.h"
#include "system.h"
#include "kangaroo_twelve.h"
#include "common_buffers.h"


static volatile char spectrumLock = 0;
static ::Entity* spectrum = nullptr;
static struct SpectrumInfo {
    unsigned int numberOfEntities = 0;  // Number of entities in the spectrum hash map, may include entries with balance == 0
    unsigned long long totalAmount = 0; // Total amount of qubics in the spectrum
} spectrumInfo;

static unsigned int entityCategoryPopulations[48]; // Array size depends on max possible balance
static constexpr unsigned char entityCategoryCount = sizeof(entityCategoryPopulations) / sizeof(entityCategoryPopulations[0]);
unsigned long long dustThresholdBurnAll = 0, dustThresholdBurnHalf = 0;

static m256i* spectrumDigests = nullptr;
constexpr unsigned long long spectrumDigestsSizeInByte = (SPECTRUM_CAPACITY * 2 - 1) * 32ULL;

static unsigned long long spectrumReorgTotalExecutionTicks = 0;


// Update SpectrumInfo data (exensive, because it iterates the whole spectrum), acquire no lock
void updateSpectrumInfo(SpectrumInfo& si = spectrumInfo)
{
    si.numberOfEntities = 0;
    si.totalAmount = 0;
    for (unsigned int i = 0; i < SPECTRUM_CAPACITY; i++)
    {
        long long balance = spectrum[i].incomingAmount - spectrum[i].outgoingAmount;
        if (balance || !isZero(spectrum[i].publicKey))
        {
            si.numberOfEntities++;
            si.totalAmount += balance;
        }
    }
}

// Compute balances that count as dust and are burned if 75% of spectrum hash map is filled.
// All balances <= dustThresholdBurnAll are burned in this case.
// Every 2nd balance <= dustThresholdBurnHalf is burned in this case.
void updateAndAnalzeEntityCategoryPopulations()
{
    static_assert(MAX_SUPPLY < (1llu << entityCategoryCount));
    setMem(entityCategoryPopulations, sizeof(entityCategoryPopulations), 0);

    for (unsigned int i = 0; i < SPECTRUM_CAPACITY; i++)
    {
        const unsigned long long balance = spectrum[i].incomingAmount - spectrum[i].outgoingAmount;
        if (balance)
        {
            entityCategoryPopulations[63 - __lzcnt64(balance)]++;
        }
    }

    dustThresholdBurnAll = 0;
    dustThresholdBurnHalf = 0;
    unsigned int numberOfEntities = 0;
    for (unsigned int categoryIndex = entityCategoryCount; categoryIndex-- > 0; )
    {
        if ((numberOfEntities += entityCategoryPopulations[categoryIndex]) >= SPECTRUM_CAPACITY / 2)
        {
            // Balances in this category + higher balances fill more than half of the spectrum
            // -> Reduce to less than half of the spectrum by detelting this category + lower balances
            if (entityCategoryPopulations[categoryIndex] == numberOfEntities)
            {
                // Corner case handling: if all entities would be deleted, burn only half in the current category
                // and all in the the lower categories
                dustThresholdBurnHalf = (1llu << (categoryIndex + 1)) - 1;
                dustThresholdBurnAll = (1llu << categoryIndex) - 1;
            }
            else
            {
                // Regular case: burn all balances in current category and smaller (reduces
                // spectrum to < 50% of capacity, but keeps as many entities as possible
                // within this contraint and the granularity of categories
                dustThresholdBurnAll = (1llu << (categoryIndex + 1)) - 1;
            }
            break;
        }
    }
}

// Clean up spectrum hash map, removing all entities with balance 0. Updates spectrumInfo.
static void reorganizeSpectrum()
{
    unsigned long long spectrumReorgStartTick = __rdtsc();

    ::Entity* reorgSpectrum = (::Entity*)reorgBuffer;
    setMem(reorgSpectrum, SPECTRUM_CAPACITY * sizeof(::Entity), 0);
    for (unsigned int i = 0; i < SPECTRUM_CAPACITY; i++)
    {
        if (spectrum[i].incomingAmount - spectrum[i].outgoingAmount)
        {
            unsigned int index = spectrum[i].publicKey.m256i_u32[0] & (SPECTRUM_CAPACITY - 1);

        iteration:
            if (isZero(reorgSpectrum[index].publicKey))
            {
                copyMem(&reorgSpectrum[index], &spectrum[i], sizeof(::Entity));
            }
            else
            {
                index = (index + 1) & (SPECTRUM_CAPACITY - 1);

                goto iteration;
            }
        }
    }
    copyMem(spectrum, reorgSpectrum, SPECTRUM_CAPACITY * sizeof(::Entity));

    unsigned int digestIndex;
    for (digestIndex = 0; digestIndex < SPECTRUM_CAPACITY; digestIndex++)
    {
        KangarooTwelve64To32(&spectrum[digestIndex], &spectrumDigests[digestIndex]);
    }
    unsigned int previousLevelBeginning = 0;
    unsigned int numberOfLeafs = SPECTRUM_CAPACITY;
    while (numberOfLeafs > 1)
    {
        for (unsigned int i = 0; i < numberOfLeafs; i += 2)
        {
            KangarooTwelve64To32(&spectrumDigests[previousLevelBeginning + i], &spectrumDigests[digestIndex++]);
        }

        previousLevelBeginning += numberOfLeafs;
        numberOfLeafs >>= 1;
    }

    updateSpectrumInfo();

    spectrumReorgTotalExecutionTicks += __rdtsc() - spectrumReorgStartTick;
}

static int spectrumIndex(const m256i& publicKey)
{
    if (isZero(publicKey))
    {
        return -1;
    }

    unsigned int index = publicKey.m256i_u32[0] & (SPECTRUM_CAPACITY - 1);

    ACQUIRE(spectrumLock);

iteration:
    if (spectrum[index].publicKey == publicKey)
    {
        RELEASE(spectrumLock);

        return index;
    }
    else
    {
        if (isZero(spectrum[index].publicKey))
        {
            RELEASE(spectrumLock);

            return -1;
        }
        else
        {
            index = (index + 1) & (SPECTRUM_CAPACITY - 1);

            goto iteration;
        }
    }
}

static long long energy(const int index)
{
    return spectrum[index].incomingAmount - spectrum[index].outgoingAmount;
}

// Increase balance of entity. Does not update spectrumInfo.totalAmount.
static void increaseEnergy(const m256i& publicKey, long long amount)
{
    if (!isZero(publicKey) && amount >= 0)
    {
        unsigned int index = publicKey.m256i_u32[0] & (SPECTRUM_CAPACITY - 1);

        ACQUIRE(spectrumLock);

        if (spectrumInfo.numberOfEntities >= (SPECTRUM_CAPACITY / 2) + (SPECTRUM_CAPACITY / 4))
        {
            // Update anti-dust burn thresholds
            updateAndAnalzeEntityCategoryPopulations();

            if (dustThresholdBurnAll > 0)
            {
                // Burn every balance with balance < dustThresholdBurnAll
                for (unsigned int i = 0; i < SPECTRUM_CAPACITY; i++)
                {
                    const unsigned long long balance = spectrum[i].incomingAmount - spectrum[i].outgoingAmount;
                    if (balance <= dustThresholdBurnAll && balance)
                    {
                        spectrum[i].outgoingAmount = spectrum[i].incomingAmount;
                    }
                }
            }

            if (dustThresholdBurnHalf > 0)
            {
                // Burn every second balance with balance < dustThresholdBurnHalf
                unsigned int countBurnCanadiates = 0;
                for (unsigned int i = 0; i < SPECTRUM_CAPACITY; i++)
                {
                    const unsigned long long balance = spectrum[i].incomingAmount - spectrum[i].outgoingAmount;
                    if (balance <= dustThresholdBurnHalf && balance)
                    {
                        if (++countBurnCanadiates & 1)
                        {
                            spectrum[i].outgoingAmount = spectrum[i].incomingAmount;
                        }
                    }
                }
            }

            // Remove entries with balance zero from hash map
            reorganizeSpectrum();

            // Correct total amount (spectrum info has been recomputed before increasing energy;
            // in transfer case energy has been decreased before and total amount is not changed
            // without anti-dust burning)
            spectrumInfo.totalAmount += amount;
        }

    iteration:
        if (spectrum[index].publicKey == publicKey)
        {
            spectrum[index].incomingAmount += amount;
            spectrum[index].numberOfIncomingTransfers++;
            spectrum[index].latestIncomingTransferTick = system.tick;
        }
        else
        {
            if (isZero(spectrum[index].publicKey))
            {
                spectrum[index].publicKey = publicKey;
                spectrum[index].incomingAmount = amount;
                spectrum[index].numberOfIncomingTransfers = 1;
                spectrum[index].latestIncomingTransferTick = system.tick;

                spectrumInfo.numberOfEntities++;
            }
            else
            {
                index = (index + 1) & (SPECTRUM_CAPACITY - 1);

                goto iteration;
            }
        }

        RELEASE(spectrumLock);
    }
}

// Decrease balance of entity if it is high enough. Does not update spectrumInfo.totalAmount.
static bool decreaseEnergy(const int index, long long amount)
{
    if (amount >= 0)
    {
        ACQUIRE(spectrumLock);

        if (energy(index) >= amount)
        {
            spectrum[index].outgoingAmount += amount;
            spectrum[index].numberOfOutgoingTransfers++;
            spectrum[index].latestOutgoingTransferTick = system.tick;

            RELEASE(spectrumLock);

            return true;
        }

        RELEASE(spectrumLock);
    }

    return false;
}


static bool loadSpectrum(const CHAR16* fileName = SPECTRUM_FILE_NAME, const CHAR16* directory = nullptr)
{
    logToConsole(L"Loading spectrum file ...");
    long long loadedSize = load(fileName, SPECTRUM_CAPACITY * sizeof(::Entity), (unsigned char*)spectrum, directory);
    if (loadedSize != SPECTRUM_CAPACITY * sizeof(::Entity))
    {
        logStatusToConsole(L"EFI_FILE_PROTOCOL.Read() reads invalid number of bytes", loadedSize, __LINE__);

        return false;
    }
    updateSpectrumInfo();
    return true;
}

static bool saveSpectrum(const CHAR16* fileName = SPECTRUM_FILE_NAME, const CHAR16* directory = nullptr)
{
    logToConsole(L"Saving spectrum file...");

    const unsigned long long beginningTick = __rdtsc();

    ACQUIRE(spectrumLock);
    long long savedSize = save(fileName, SPECTRUM_CAPACITY * sizeof(::Entity), (unsigned char*)spectrum, directory);
    RELEASE(spectrumLock);

    if (savedSize == SPECTRUM_CAPACITY * sizeof(::Entity))
    {
        setNumber(message, savedSize, TRUE);
        appendText(message, L" bytes of the spectrum data are saved (");
        appendNumber(message, (__rdtsc() - beginningTick) * 1000000 / frequency, TRUE);
        appendText(message, L" microseconds).");
        logToConsole(message);
        return true;
    }
    return false;
}

static bool initSpectrum()
{
    if (!allocatePool(spectrumSizeInBytes, (void**)&spectrum)
        || !allocatePool(spectrumDigestsSizeInByte, (void**)&spectrumDigests))
    {
        logToConsole(L"Failed to allocate spectrum memory!");
        return false;
    }

    return true;
}

static void deinitSpectrum()
{
    if (spectrumDigests)
    {
        freePool(spectrumDigests);
    }
    if (spectrum)
    {
        freePool(spectrum);
    }
}
