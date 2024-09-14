using namespace QPI;

constexpr uint64 MINIMUM_LOCKING_AMOUNT = 10000000;
constexpr uint64 MAXIMUM_OF_USER = 16777216;
constexpr uint64 MAXIMUM_EPOCH = 65535;
constexpr uint64 MAXIMUM_UNLOCK_HISTORY = 524288;
constexpr uint64 INITIAL_EPOCH = 999;                             //  we need to change this epoch when merging

constexpr uint64 EARLY_UNLOCKING_PERCENT_0_3 = 0;
constexpr uint64 EARLY_UNLOCKING_PERCENT_4_7 = 5;
constexpr uint64 EARLY_UNLOCKING_PERCENT_8_11 = 5;
constexpr uint64 EARLY_UNLOCKING_PERCENT_12_15 = 10;
constexpr uint64 EARLY_UNLOCKING_PERCENT_16_19 = 15;
constexpr uint64 EARLY_UNLOCKING_PERCENT_20_23 = 20;
constexpr uint64 EARLY_UNLOCKING_PERCENT_24_27 = 25;
constexpr uint64 EARLY_UNLOCKING_PERCENT_28_31 = 30;
constexpr uint64 EARLY_UNLOCKING_PERCENT_32_35 = 35;
constexpr uint64 EARLY_UNLOCKING_PERCENT_36_39 = 40;
constexpr uint64 EARLY_UNLOCKING_PERCENT_40_43 = 45;
constexpr uint64 EARLY_UNLOCKING_PERCENT_44_47 = 50;
constexpr uint64 EARLY_UNLOCKING_PERCENT_48_51 = 55;

constexpr uint64 BURN_PERCENT_0_3 = 0;
constexpr uint64 BURN_PERCENT_4_7 = 45;
constexpr uint64 BURN_PERCENT_8_11 = 45;
constexpr uint64 BURN_PERCENT_12_15 = 45;
constexpr uint64 BURN_PERCENT_16_19 = 40;
constexpr uint64 BURN_PERCENT_20_23 = 40;
constexpr uint64 BURN_PERCENT_24_27 = 35;
constexpr uint64 BURN_PERCENT_28_31 = 35;
constexpr uint64 BURN_PERCENT_32_35 = 35;
constexpr uint64 BURN_PERCENT_36_39 = 30;
constexpr uint64 BURN_PERCENT_40_43 = 30;
constexpr uint64 BURN_PERCENT_44_47 = 30;
constexpr uint64 BURN_PERCENT_48_51 = 25;

constexpr sint32 INVALID_INPUT_AMOUNT = -1;
constexpr sint32 LOCK_SUCCESS = 0;
constexpr sint32 INVALID_INPUT_LOCKED_EPOCH = 1;
constexpr sint32 INVALID_INPUT_UNLOCK_AMOUNT = 2;
constexpr sint32 EMPTY_LOCKED = 3;
constexpr sint32 UNLOCK_SUCCESS = 4;
constexpr sint32 OVERFLOW_USER = 5;

struct QEARN2
{
};

struct QEARN : public ContractBase
{
public:
    struct GetLockInforPerEpoch_input {
		uint32 Epoch;                             /* epoch number to get information */
    };

    struct GetLockInforPerEpoch_output {
        uint64 LockedAmount;                      /* initial total locked amount in epoch */
        uint64 BonusAmount;                       /* initial bonus amount in epoch*/
        uint64 CurrentLockedAmount;               /* total locked amount in epoch. exactly the amount excluding the amount unlocked early*/
        uint64 CurrentBonusAmount;                /* bonus amount in epoch excluding the early unlocking */
        uint64 Yield;                             /* Yield calculated by 10000000 multiple*/
    };

    struct GetUserLockedInfor_input {
        id user;
        uint32 epoch;
    };

    struct GetUserLockedInfor_output {
        uint64 LockedAmount;                   /* the amount user locked at input.epoch */
    };

    struct GetStateOfRound_input {
        uint32 Epoch;
    };

    struct GetStateOfRound_output {
        uint32 state;
    };
    
    struct GetUserLockStatus_input {
        id user;
    };

    struct GetUserLockStatus_output {
        uint64 status;
    };

    struct GetEndedStatus_input {
        id user;
    };

    struct GetEndedStatus_output {
        uint64 Fully_Unlocked_Amount;
        uint64 Fully_Rewarded_Amount;
        uint64 Early_Unlocked_Amount;
        uint64 Early_Rewarded_Amount;
    };

	struct Lock_input {	
    };

    struct Lock_output {	
        sint32 returnCode;
    };

    struct Unlock_input {
        uint64 Amount;                            /* unlocking amount */	
        uint32 Locked_Epoch;                      /* locked epoch */
    };

    struct Unlock_output {
        sint32 returnCode;
    };

private:

    struct RoundInfo {

        uint64 _Total_Locked_Amount;            // The initial total locked amount in any epoch.  Max Epoch is 65535
        uint64 _Epoch_Bonus_Amount;             // The initial bonus amount per an epoch.         Max Epoch is 65535 

    };

    array<RoundInfo, 65536> _InitialRoundInfo;
    array<RoundInfo, 65536> _CurrentRoundInfo;

    struct EpochIndexInfo {

        uint32 startIndex;
        uint32 endIndex;
    };

    array<EpochIndexInfo, 65536> EpochIndex;

    struct UserInfo {

        uint64 _Locked_Amount;
        id ID;
        uint32 _Locked_Epoch;
        
    };

    array<UserInfo, 16777216> Locker;

    struct HistoryInfo {

        uint64 _Unlocked_Amount;
        uint64 _Rewarded_Amount;
        id _Unlocked_ID;

    };

    array<HistoryInfo, 524288> EarlyUnlocker;
    array<HistoryInfo, 524288> FullyUnlocker;
    
    uint64 remain_amount;          //  The remain amount boosted by last early unlocker of one round, this amount will put in next round

    uint32 _EarlyUnlocked_cnt;
    uint32 _FullyUnlocked_cnt;

    PUBLIC_FUNCTION(GetStateOfRound)
        if(input.Epoch < INITIAL_EPOCH) 
        {                                                            // non staking
            output.state = 2; return ;
        }
        if(input.Epoch > qpi.epoch()) output.state = 0;                                     // opening round, not started yet
        if(input.Epoch <= qpi.epoch() && input.Epoch >= qpi.epoch() - 52) output.state = 1; // running round, available unlocking early
        if(input.Epoch < qpi.epoch() - 52) output.state = 2;                                // ended round
    _

    PUBLIC_FUNCTION(GetLockInforPerEpoch)

        output.BonusAmount = state._InitialRoundInfo.get(input.Epoch)._Epoch_Bonus_Amount;
        output.LockedAmount = state._InitialRoundInfo.get(input.Epoch)._Total_Locked_Amount;
        output.CurrentBonusAmount = state._CurrentRoundInfo.get(input.Epoch)._Epoch_Bonus_Amount;
        output.CurrentLockedAmount = state._CurrentRoundInfo.get(input.Epoch)._Total_Locked_Amount;
        if(state._CurrentRoundInfo.get(input.Epoch)._Total_Locked_Amount) 
        {
            output.Yield = state._CurrentRoundInfo.get(input.Epoch)._Epoch_Bonus_Amount * 10000000UL / state._CurrentRoundInfo.get(input.Epoch)._Total_Locked_Amount;
        }
        else output.Yield = 0UL;
    _

    struct GetUserLockedInfor_locals {
        uint32 _t;
    };

    PUBLIC_FUNCTION_WITH_LOCALS(GetUserLockedInfor)
        
        for(locals._t = state.EpochIndex.get(input.epoch).startIndex; locals._t < state.EpochIndex.get(input.epoch).endIndex; locals._t++) 
        {
            if(state.Locker.get(locals._t).ID == input.user) 
            {
                output.LockedAmount = state.Locker.get(locals._t)._Locked_Amount; return;
            }
        }
    _

    struct GetUserLockStatus_locals {
        uint64 bn;
        uint32 _t;
        uint32 _r;
        uint8 lockedWeeks;
    };

    PUBLIC_FUNCTION_WITH_LOCALS(GetUserLockStatus)

        output.status = 0UL;
        
        for(locals._t = 0; locals._t < state.EpochIndex.get(qpi.epoch()).endIndex; locals._t++) 
        {
            if(state.Locker.get(locals._t).ID == input.user && state.Locker.get(locals._t)._Locked_Amount > 0) 
            {
            
                locals.lockedWeeks = qpi.epoch() - state.Locker.get(locals._t)._Locked_Epoch;
                locals.bn = 1;

                for(locals._r = 0; locals._r < locals.lockedWeeks; locals._r++) locals.bn *= 2;

                output.status += locals.bn;
            }
        }

    _
    
    struct GetEndedStatus_locals {
        uint32 _t;
    };

    PUBLIC_FUNCTION_WITH_LOCALS(GetEndedStatus)

        output.Early_Rewarded_Amount = 0;
        output.Early_Unlocked_Amount = 0;
        output.Fully_Rewarded_Amount = 0;
        output.Fully_Unlocked_Amount = 0;

        for(locals._t = 0; locals._t < state._EarlyUnlocked_cnt; locals._t++) 
        {
            if(state.EarlyUnlocker.get(locals._t)._Unlocked_ID == input.user) 
            {
                output.Early_Rewarded_Amount = state.EarlyUnlocker.get(locals._t)._Rewarded_Amount;
                output.Early_Unlocked_Amount = state.EarlyUnlocker.get(locals._t)._Unlocked_Amount;

                break ;
            }
        }

        for(locals._t = 0; locals._t < state._FullyUnlocked_cnt; locals._t++) 
        {
            if(state.FullyUnlocker.get(locals._t)._Unlocked_ID == input.user) 
            {
                output.Fully_Rewarded_Amount = state.FullyUnlocker.get(locals._t)._Rewarded_Amount;
                output.Fully_Unlocked_Amount = state.FullyUnlocker.get(locals._t)._Unlocked_Amount;
            
                return ;
            }
        }
    _

    struct Lock_locals {

        UserInfo newLocker;
        RoundInfo updatedRoundInfo;
        EpochIndexInfo tmpIndex;
        sint32 t;
        
    };

    PUBLIC_PROCEDURE_WITH_LOCALS(Lock)
    
        if(qpi.invocationReward() < MINIMUM_LOCKING_AMOUNT)
        {
            output.returnCode = INVALID_INPUT_AMOUNT;         // if the amount of locking is less than 10M, it should be failed to lock.
            
            if(qpi.invocationReward() > 0) 
            {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
            }
            return;
        }

        for(locals.t = state.EpochIndex.get(qpi.epoch()).startIndex ; locals.t < state.EpochIndex.get(qpi.epoch()).endIndex; locals.t++) 
        {

            if(state.Locker.get(locals.t).ID == qpi.invocator()) 
            {      // the case to be locked several times at one epoch, at that time, this address already located in state.Locker array, the amount will be increased as current locking amount.
                
                locals.newLocker._Locked_Amount = state.Locker.get(locals.t)._Locked_Amount + qpi.invocationReward();
                locals.newLocker._Locked_Epoch = qpi.epoch();
                locals.newLocker.ID = qpi.invocator();

                state.Locker.set(locals.t, locals.newLocker);

                locals.updatedRoundInfo._Total_Locked_Amount = state._InitialRoundInfo.get(qpi.epoch())._Total_Locked_Amount + qpi.invocationReward();
                locals.updatedRoundInfo._Epoch_Bonus_Amount = state._InitialRoundInfo.get(qpi.epoch())._Epoch_Bonus_Amount;
                state._InitialRoundInfo.set(qpi.epoch(), locals.updatedRoundInfo);

                locals.updatedRoundInfo._Total_Locked_Amount = state._CurrentRoundInfo.get(qpi.epoch())._Total_Locked_Amount + qpi.invocationReward();
                locals.updatedRoundInfo._Epoch_Bonus_Amount = state._CurrentRoundInfo.get(qpi.epoch())._Epoch_Bonus_Amount;
                state._CurrentRoundInfo.set(qpi.epoch(), locals.updatedRoundInfo);
                
                output.returnCode = LOCK_SUCCESS;          //  additional locking of this epoch is succeed
                return ;
            }

        }

        if(state.EpochIndex.get(qpi.epoch()).endIndex == MAXIMUM_OF_USER - 1) 
        {
            output.returnCode = OVERFLOW_USER;
            if(qpi.invocationReward() > 0) 
            {
                qpi.transfer(qpi.invocator(), qpi.invocationReward());
            }
            return ;                        // overflow users in Qearn
        }

        locals.newLocker.ID = qpi.invocator();
        locals.newLocker._Locked_Amount = qpi.invocationReward();
        locals.newLocker._Locked_Epoch = qpi.epoch();

        state.Locker.set(state.EpochIndex.get(qpi.epoch()).endIndex, locals.newLocker);

        locals.tmpIndex.startIndex = state.EpochIndex.get(qpi.epoch()).startIndex;
        locals.tmpIndex.endIndex = state.EpochIndex.get(qpi.epoch()).endIndex + 1;
        state.EpochIndex.set(qpi.epoch(), locals.tmpIndex);

        locals.updatedRoundInfo._Total_Locked_Amount = state._InitialRoundInfo.get(qpi.epoch())._Total_Locked_Amount + qpi.invocationReward();
        locals.updatedRoundInfo._Epoch_Bonus_Amount = state._InitialRoundInfo.get(qpi.epoch())._Epoch_Bonus_Amount;
        state._InitialRoundInfo.set(qpi.epoch(), locals.updatedRoundInfo);

        locals.updatedRoundInfo._Total_Locked_Amount = state._CurrentRoundInfo.get(qpi.epoch())._Total_Locked_Amount + qpi.invocationReward();
        locals.updatedRoundInfo._Epoch_Bonus_Amount = state._CurrentRoundInfo.get(qpi.epoch())._Epoch_Bonus_Amount;
        state._CurrentRoundInfo.set(qpi.epoch(), locals.updatedRoundInfo);

        output.returnCode = LOCK_SUCCESS;            //  new locking of this epoch is succeed
    _

    struct Unlock_locals {

        RoundInfo updatedRoundInfo;
        UserInfo updatedUserInfo;
        HistoryInfo unlockerInfo;
        
        uint64 AmountOfUnlocking;
        uint64 AmountOfReward;
        uint64 AmountOfburn;
        uint64 RewardPercent;
        sint64 divCalcu;
        uint32 indexOfinvocator;
        sint32 t;
        uint32 count_Of_last_vacancy;
        uint32 locked_weeks;
        
    };

    PUBLIC_PROCEDURE_WITH_LOCALS(Unlock)

        if(input.Locked_Epoch > MAXIMUM_EPOCH) {

            output.returnCode = INVALID_INPUT_LOCKED_EPOCH;               //   if user try to unlock with wrong locked epoch, it should be failed to unlock.
            return ;

        }

        locals.indexOfinvocator = MAXIMUM_OF_USER;

        for(locals.t = state.EpochIndex.get(input.Locked_Epoch).startIndex ; locals.t < state.EpochIndex.get(input.Locked_Epoch).endIndex; locals.t++) 
        {

            if(state.Locker.get(locals.t).ID == qpi.invocator()) 
            { 
                if(state.Locker.get(locals.t)._Locked_Amount < input.Amount) {

                    output.returnCode = INVALID_INPUT_UNLOCK_AMOUNT;  //  if the amount to be wanted to unlock is more than locked amount, it should be failed to unlock
                    return ;  

                }
                else {
                    locals.indexOfinvocator = locals.t;
                    break;
                }
            }

        }

        if(locals.indexOfinvocator == MAXIMUM_OF_USER) {
            
            output.returnCode = EMPTY_LOCKED;     //   if there is no any locked info in state.Locker array, it shows this address didn't lock at the epoch (input.Locked_Epoch)
            return ;  
        }

        /* the rest amount after unlocking should be more than MINIMUM_LOCKING_AMOUNT */
        if(state.Locker.get(locals.indexOfinvocator)._Locked_Amount - input.Amount < MINIMUM_LOCKING_AMOUNT) 
        {
            locals.AmountOfUnlocking = state.Locker.get(locals.indexOfinvocator)._Locked_Amount;
        }
        else locals.AmountOfUnlocking = input.Amount;

        locals.RewardPercent = QPI::div(state._CurrentRoundInfo.get(input.Locked_Epoch)._Epoch_Bonus_Amount * 10000000ULL, state._CurrentRoundInfo.get(input.Locked_Epoch)._Total_Locked_Amount);
        locals.locked_weeks = qpi.epoch() - input.Locked_Epoch - 1;
        locals.divCalcu = QPI::div(locals.RewardPercent * locals.AmountOfUnlocking , 100ULL);

        if(qpi.epoch() - input.Locked_Epoch >= 0 && locals.locked_weeks <= 3) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_0_3, 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_0_3 , 10000000ULL);
        }

        if(locals.locked_weeks >= 4 && locals.locked_weeks <= 7) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_4_7 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_4_7 , 10000000ULL);
        }

        if(locals.locked_weeks >= 8 && locals.locked_weeks <= 11) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_8_11 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_8_11 , 10000000ULL);
        }

        if(locals.locked_weeks >= 12 && locals.locked_weeks <= 15) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_12_15 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_12_15 , 10000000ULL);
        }

        if(locals.locked_weeks >= 16 && locals.locked_weeks <= 19) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_16_19 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_16_19 , 10000000ULL);
        }

        if(locals.locked_weeks >= 20 && locals.locked_weeks <= 23) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_20_23 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_20_23 , 10000000ULL);
        }

        if(locals.locked_weeks >= 24 && locals.locked_weeks <= 27) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_24_27 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_24_27 , 10000000ULL);
        }

        if(locals.locked_weeks >= 28 && locals.locked_weeks <= 31) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_28_31 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_28_31 , 10000000ULL);
        }

        if(locals.locked_weeks >= 32 && locals.locked_weeks <= 35) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_32_35 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_32_35 , 10000000ULL);
        }

        if(locals.locked_weeks >= 36 && locals.locked_weeks <= 39) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_36_39 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_36_39 , 10000000ULL);
        }

        if(locals.locked_weeks >= 40 && locals.locked_weeks <= 43) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_40_43 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_40_43 , 10000000ULL);
        }

        if(locals.locked_weeks >= 44 && locals.locked_weeks <= 47) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_44_47 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_44_47 , 10000000ULL);
        }

        if(locals.locked_weeks >= 48 && locals.locked_weeks <= 51) 
        {
            locals.AmountOfReward = QPI::div(locals.divCalcu * EARLY_UNLOCKING_PERCENT_48_51 , 10000000ULL);
            locals.AmountOfburn = QPI::div(locals.divCalcu * BURN_PERCENT_48_51 , 10000000ULL);
        }

        qpi.transfer(qpi.invocator(), locals.AmountOfUnlocking + locals.AmountOfReward);
        qpi.burn(locals.AmountOfburn);

        locals.updatedRoundInfo._Total_Locked_Amount = state._CurrentRoundInfo.get(input.Locked_Epoch)._Total_Locked_Amount - locals.AmountOfUnlocking;
        locals.updatedRoundInfo._Epoch_Bonus_Amount = state._CurrentRoundInfo.get(input.Locked_Epoch)._Epoch_Bonus_Amount - locals.AmountOfReward - locals.AmountOfburn;

        state._CurrentRoundInfo.set(input.Locked_Epoch, locals.updatedRoundInfo);

        locals.updatedUserInfo.ID = qpi.invocator();
        locals.updatedUserInfo._Locked_Amount = state.Locker.get(locals.indexOfinvocator)._Locked_Amount - locals.AmountOfUnlocking;
        locals.updatedUserInfo._Locked_Epoch = state.Locker.get(locals.indexOfinvocator)._Locked_Epoch;

        state.Locker.set(locals.indexOfinvocator, locals.updatedUserInfo);

        if(state._CurrentRoundInfo.get(input.Locked_Epoch)._Total_Locked_Amount == 0 && input.Locked_Epoch != qpi.epoch()) 
        {   // The case to be unlocked early all users of one epoch, at this time, boost amount will put in next round.
            
            state.remain_amount = state._CurrentRoundInfo.get(input.Locked_Epoch)._Epoch_Bonus_Amount;

            locals.updatedRoundInfo._Total_Locked_Amount = 0;
            locals.updatedRoundInfo._Epoch_Bonus_Amount = 0;

            state._CurrentRoundInfo.set(input.Locked_Epoch, locals.updatedRoundInfo);

        }

        if(input.Locked_Epoch != qpi.epoch()) 
        {

            locals.unlockerInfo._Unlocked_ID = qpi.invocator();
            
            for(locals.t = 0; locals.t < state._EarlyUnlocked_cnt; locals.t++) 
            {
                if(state.EarlyUnlocker.get(locals.t)._Unlocked_ID == qpi.invocator()) 
                {

                    locals.unlockerInfo._Rewarded_Amount = state.EarlyUnlocker.get(locals.t)._Rewarded_Amount + locals.AmountOfReward;
                    locals.unlockerInfo._Unlocked_Amount = state.EarlyUnlocker.get(locals.t)._Unlocked_Amount + locals.AmountOfUnlocking;

                    state.EarlyUnlocker.set(locals.t, locals.unlockerInfo);

                    break;
                }
            }

            if(locals.t == state._EarlyUnlocked_cnt && state._EarlyUnlocked_cnt < MAXIMUM_UNLOCK_HISTORY) 
            {
                locals.unlockerInfo._Rewarded_Amount = locals.AmountOfReward;
                locals.unlockerInfo._Unlocked_Amount = locals.AmountOfUnlocking;

                state.EarlyUnlocker.set(locals.t, locals.unlockerInfo);
                state._EarlyUnlocked_cnt++;
            }

        }

        output.returnCode = UNLOCK_SUCCESS; //  unlock is succeed
    _

	REGISTER_USER_FUNCTIONS_AND_PROCEDURES
    
        REGISTER_USER_FUNCTION(GetLockInforPerEpoch, 1);
        REGISTER_USER_FUNCTION(GetUserLockedInfor, 2);
        REGISTER_USER_FUNCTION(GetStateOfRound, 3);
        REGISTER_USER_FUNCTION(GetUserLockStatus, 4);
        REGISTER_USER_FUNCTION(GetEndedStatus, 5);

        REGISTER_USER_PROCEDURE(Lock, 1);
		REGISTER_USER_PROCEDURE(Unlock, 2);

	_

    struct BEGIN_EPOCH_locals
    {
        HistoryInfo INITIALIZE_HISTORY;
        UserInfo INITIALIZE_USER;
        RoundInfo INITIALIZE_ROUNDINFO;

        uint32 t;
        bit status;
        uint64 pre_epoch_balance;
        uint64 current_balance;
        ::Entity entity;
        uint32 locked_epoch;
    };

    BEGIN_EPOCH_WITH_LOCALS

        qpi.getEntity(SELF, locals.entity);
        locals.current_balance = locals.entity.incomingAmount - locals.entity.outgoingAmount;

        locals.pre_epoch_balance = 0UL;
        locals.locked_epoch = qpi.epoch() - 52;
        for(locals.t = qpi.epoch() - 1; locals.t >= locals.locked_epoch; locals.t--) 
        {
            locals.pre_epoch_balance += state._CurrentRoundInfo.get(locals.t)._Epoch_Bonus_Amount + state._CurrentRoundInfo.get(locals.t)._Total_Locked_Amount;
        }

        locals.INITIALIZE_ROUNDINFO._Epoch_Bonus_Amount = locals.current_balance - locals.pre_epoch_balance;
        locals.INITIALIZE_ROUNDINFO._Total_Locked_Amount = 0;

        state._InitialRoundInfo.set(qpi.epoch(), locals.INITIALIZE_ROUNDINFO);
        state._CurrentRoundInfo.set(qpi.epoch(), locals.INITIALIZE_ROUNDINFO);

        if(locals.INITIALIZE_ROUNDINFO._Epoch_Bonus_Amount > 0) 
        {
            
            locals.INITIALIZE_ROUNDINFO._Epoch_Bonus_Amount += state.remain_amount;

            state._InitialRoundInfo.set(qpi.epoch(), locals.INITIALIZE_ROUNDINFO);
            state._CurrentRoundInfo.set(qpi.epoch(), locals.INITIALIZE_ROUNDINFO);

            state.remain_amount = 0;

        }
	_

    struct END_EPOCH_locals 
    {
        HistoryInfo INITIALIZE_HISTORY;
        UserInfo INITIALIZE_USER;
        RoundInfo INITIALIZE_ROUNDINFO;
        EpochIndexInfo tmpEpochIndex;

        uint64 _reward_percent;
        uint64 _reward_amount;
        uint64 _burn_amount;
        uint32 locked_epoch;
        uint32 startEpoch;
        sint32 _t;
        sint32 st;
        sint32 en;
        uint32 empty_cnt;

    };

	END_EPOCH_WITH_LOCALS

        state._EarlyUnlocked_cnt = 0;
        state._FullyUnlocked_cnt = 0;
        locals.locked_epoch = qpi.epoch() - 52;
        
        locals._burn_amount = state._CurrentRoundInfo.get(locals.locked_epoch)._Epoch_Bonus_Amount;
        
        locals._reward_percent = QPI::div(state._CurrentRoundInfo.get(locals.locked_epoch)._Epoch_Bonus_Amount * 10000000ULL, state._CurrentRoundInfo.get(locals.locked_epoch)._Total_Locked_Amount);

        for(locals._t = state.EpochIndex.get(locals.locked_epoch).startIndex; locals._t < state.EpochIndex.get(locals.locked_epoch).endIndex; locals._t++) 
        {
            if(state.Locker.get(locals._t)._Locked_Amount == 0) continue;

            locals._reward_amount = QPI::div(state.Locker.get(locals._t)._Locked_Amount * locals._reward_percent, 10000000ULL);
            qpi.transfer(state.Locker.get(locals._t).ID, locals._reward_amount + state.Locker.get(locals._t)._Locked_Amount);

            if(state._FullyUnlocked_cnt < MAXIMUM_UNLOCK_HISTORY) 
            {

                locals.INITIALIZE_HISTORY._Unlocked_ID = state.Locker.get(locals._t).ID;
                locals.INITIALIZE_HISTORY._Unlocked_Amount = state.Locker.get(locals._t)._Locked_Amount;
                locals.INITIALIZE_HISTORY._Rewarded_Amount = locals._reward_amount;

                state.FullyUnlocker.set(state._FullyUnlocked_cnt, locals.INITIALIZE_HISTORY);

                state._FullyUnlocked_cnt++;
            }

            locals.INITIALIZE_USER.ID = NULL_ID;
            locals.INITIALIZE_USER._Locked_Amount = 0;
            locals.INITIALIZE_USER._Locked_Epoch = 0;

            state.Locker.set(locals._t, locals.INITIALIZE_USER);

            locals._burn_amount -= locals._reward_amount;
        }

        locals.st = 0;

        for(locals._t = 0; locals._t < MAXIMUM_OF_USER; locals._t++)
        {
            if(state.Locker.get(locals._t)._Locked_Epoch) 
            {
                locals.startEpoch = state.Locker.get(locals._t)._Locked_Epoch;
                break;
            }
        }

        for(locals._t = locals.startEpoch; locals._t <= qpi.epoch(); locals._t++)
        {
            locals.empty_cnt = 0;
            if(locals._t == locals.startEpoch) 
            {
                locals.st = 0;
            }
            else 
            {
                locals.st = state.EpochIndex.get(locals._t).startIndex;
            }
            locals.en = state.EpochIndex.get(locals._t).endIndex - 1;

            while(locals.st < locals.en)
            {
                while(state.Locker.get(locals.st)._Locked_Amount)
                {
                    locals.st++;
                }

                while(!state.Locker.get(locals.en)._Locked_Amount)
                {
                    locals.en--;
                    locals.empty_cnt++;
                }

                locals.INITIALIZE_USER.ID = state.Locker.get(locals.en).ID;
                locals.INITIALIZE_USER._Locked_Amount = state.Locker.get(locals.en)._Locked_Amount;
                locals.INITIALIZE_USER._Locked_Epoch = state.Locker.get(locals.en)._Locked_Epoch;

                state.Locker.set(locals.st, locals.INITIALIZE_USER);

                locals.INITIALIZE_USER.ID = NULL_ID;
                locals.INITIALIZE_USER._Locked_Amount = 0;
                locals.INITIALIZE_USER._Locked_Epoch = 0;

                state.Locker.set(locals.en, locals.INITIALIZE_USER);

                locals.st++;
                locals.en--;
                locals.empty_cnt++;
            }

            if(locals.st == locals.en)
            {
                locals.empty_cnt++;
            }

            locals.tmpEpochIndex.startIndex = state.EpochIndex.get(locals._t).startIndex;
            locals.tmpEpochIndex.endIndex = state.EpochIndex.get(locals._t).endIndex - locals.empty_cnt;

            state.EpochIndex.set(locals._t, locals.tmpEpochIndex);

            locals.tmpEpochIndex.startIndex = state.EpochIndex.get(locals._t).endIndex - locals.empty_cnt;
            locals.tmpEpochIndex.endIndex = state.EpochIndex.get(locals._t + 1).endIndex;

            state.EpochIndex.set(locals._t + 1, locals.tmpEpochIndex);
            
        }

        locals.tmpEpochIndex.startIndex = state.EpochIndex.get(qpi.epoch()).endIndex - locals.empty_cnt;
        locals.tmpEpochIndex.endIndex = state.EpochIndex.get(qpi.epoch()).endIndex - locals.empty_cnt;

        state.EpochIndex.set(qpi.epoch() + 1, locals.tmpEpochIndex);

        qpi.burn(locals._burn_amount);
	_
};