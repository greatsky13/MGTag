/*
 * MessageEngine.h
 *
 *  Created on: Sep 20, 2010
 *      Author: root
 */

#ifndef MESSAGEENGINE_H_
#define MESSAGEENGINE_H_

class MessageEngine {
public:
	enum MessageType { INFO = 1, WARNING, ERR , DEFAULT};
	MessageEngine();
	virtual ~MessageEngine();
	static void showMessage(char* msg, MessageType type = DEFAULT);
};

#endif /* MESSAGEENGINE_H_ */
