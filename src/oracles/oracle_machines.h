struct OracleReplyCommitTransaction : public Transaction
{
	static constexpr unsigned char transactionType()
	{
		return 3; // TODO: Set actual value
	}

	unsigned long long queryIndex;
	m256i replyDigest;
	m256i replyKnowledgeProof;
	unsigned char signature[SIGNATURE_SIZE];
};

struct OracleReplyRevealTransactionPrefix : public Transaction
{
	static constexpr unsigned char transactionType()
	{
		return 4; // TODO: Set actual value
	}

	unsigned long long queryIndex;
};

struct OracleReplyRevealTransactionPostfix
{
	unsigned char signature[SIGNATURE_SIZE];
};