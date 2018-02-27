#include "message_processor_bob_20180221.h"

void MessageProcessorBob::initialize(void) {

	outputSignals[0]->setSymbolPeriod(inputSignals[0]->getSymbolPeriod());
	outputSignals[0]->setSamplingPeriod(inputSignals[0]->getSamplingPeriod());
	outputSignals[0]->setSamplesPerSymbol(inputSignals[0]->getSamplesPerSymbol());
	outputSignals[0]->setFirstValueToBeSaved(inputSignals[0]->getFirstValueToBeSaved());

	outputSignals[1]->setSymbolPeriod(inputSignals[0]->getSymbolPeriod());
	outputSignals[1]->setSamplingPeriod(inputSignals[0]->getSamplingPeriod());
	outputSignals[1]->setSamplesPerSymbol(inputSignals[0]->getSamplesPerSymbol());
	outputSignals[1]->setFirstValueToBeSaved(inputSignals[0]->getFirstValueToBeSaved());

}

bool MessageProcessorBob::runBlock(void) {
	bool alive{ false };

	
		alive = ProcessBasisToStore();
		alive = alive || ProcessMessageToSend();
		alive = alive || ProcessReceivedMessage();
		alive = alive || ProcessStoredMessage();
	

	return alive;
}

bool MessageProcessorBob::ProcessBasisToStore() {
	bool alive{ false };
	int ready = inputSignals[0]->ready();

	if (ready <= 0);
	else {
		if (numberOfStoredBasis < maxOfStoredBasis) {
			t_real basisIn;
			inputSignals[0]->bufferGet(&basisIn);
			storedBasis.push_back((int)basisIn);
			numberOfStoredBasis++;
			alive = true;
		}
	}

	return alive;
}

bool MessageProcessorBob::ProcessMessageToSend() {
	bool alive{ false };

	int space = outputSignals[1]->space();
	if (space <= 0) return alive;

	if (numberOfStoredBasis >= messageDataLength) {
		int process = min(space, messageDataLength);

		string mDataToSend{ "" };
		for (auto k = 0; k < process; k++) {
			mDataToSend.append(to_string(storedBasis[k]));
			//outputSignals[0]->bufferPut((t_real)storedBasis[k]);
		}

		storedBasis.erase(storedBasis.begin(),storedBasis.begin() + process);
		numberOfStoredBasis = (int)storedBasis.size();

		t_message messageToSend;
		
		messageToSend.messageData = mDataToSend;
		messageToSend.messageDataLength =to_string((t_message_data_length)mDataToSend.size());
		messageToSend.messageType = BasisReconciliation;

		outputSignals[1]->bufferPut((t_message)messageToSend);
		alive = true;
	}

	return alive;
}

bool MessageProcessorBob::ProcessReceivedMessage() {

	bool alive{ false };

	int ready = inputSignals[1]->ready();
	if (ready <= 0) return alive;
	else {
		if (numberOfStoredMessages < maxNumberOfStoredMessages) {
			t_message mIn;
			inputSignals[1]->bufferGet(&mIn);
			storedMessages.push_back(mIn);
			numberOfStoredMessages++;
			alive = true;
		}
	}
	return alive;
}

bool MessageProcessorBob::ProcessStoredMessage() {

	bool alive{ false };

	for(auto n = 0; n < numberOfStoredMessages; n++) {

			t_message_type mType = getMessageType(storedMessages[n]);
			t_message_data_length mDataLength = getMessageDataLength(storedMessages[n]);
			t_message_data mData = getMessageData(storedMessages[n], mDataLength);

			string mDataOut{ "" };
			int process{ 0 };
			switch (mType) {

			case BasisReconciliation:
				process = min(outputSignals[0]->space(), mDataLength);

				for (auto k = 0; k < process; k++) {

					alive = true;
					outputSignals[0]->bufferPut((t_binary)mData[k]);

				}
				break;
			}

			int dLength = mDataLength - process;
			mData.erase(mData.begin(), mData.begin() + process);
			if (dLength == 0) {
				storedMessages.erase(storedMessages.begin() + n);
				numberOfStoredMessages = (int)storedMessages.size();
			}
			else {
				storedMessages[n].messageDataLength = to_string(dLength);
				string mDataUpdated{ "" };
				for (unsigned int m = 0; m < mData.size(); m++) {
					mDataUpdated.append(to_string(mData[m]));
				}
				storedMessages[n].messageData = mDataUpdated;
			}

	}
	
	return alive;
}

t_message_data MessageProcessorBob::getMessageData(const t_message& msg, t_message_data_length dataLength) {

	string mDataString = msg.messageData;

	vector <int> mDataVector;

	for (int k = 0; k < dataLength; k++) {
		char data = mDataString.at(k);

		if (data == '1')
			mDataVector.push_back(1);
		else if (data == '0')
			mDataVector.push_back(0);
	}

	return mDataVector;
}

t_message_data_length MessageProcessorBob::getMessageDataLength(const t_message& msg) {

	if ((msg.messageDataLength).size() >= 0)
		return stoi(msg.messageDataLength);
	else
		return 0;
}

t_message_type MessageProcessorBob::getMessageType(const t_message& msg) {
	if ((msg.messageType).compare("BasisReconcilitation") == 0) return BasisReconciliation;

	return (t_message_type)0;
}
