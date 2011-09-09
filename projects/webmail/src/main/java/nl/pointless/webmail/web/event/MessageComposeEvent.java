package nl.pointless.webmail.web.event;

import nl.pointless.webmail.dto.Message;

/**
 * Message compose event.
 * 
 * @author Peter Postma
 */
public class MessageComposeEvent {

	private final Message message;

	/**
	 * Constructor.
	 * 
	 * @param message The message template.
	 */
	public MessageComposeEvent(Message message) {
		this.message = message;
	}

	/**
	 * @return the message template
	 */
	public Message getMessage() {
		return this.message;
	}
}
