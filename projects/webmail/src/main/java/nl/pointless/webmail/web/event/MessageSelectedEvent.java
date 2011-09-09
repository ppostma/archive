package nl.pointless.webmail.web.event;

import nl.pointless.webmail.dto.Message;

/**
 * Message selected event.
 * 
 * @author Peter Postma
 */
public class MessageSelectedEvent {

	private final Message message;

	/**
	 * Constructor.
	 * 
	 * @param message The selected message.
	 */
	public MessageSelectedEvent(Message message) {
		this.message = message;
	}

	/**
	 * @return the selected message
	 */
	public Message getMessage() {
		return this.message;
	}
}
