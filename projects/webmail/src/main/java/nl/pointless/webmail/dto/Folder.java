package nl.pointless.webmail.dto;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.StringUtils;

/**
 * Lightweight object for a mail folder.
 * 
 * @author Peter Postma
 */
public class Folder implements Serializable {

	private static final long serialVersionUID = 1L;

	private String name;
	private String fullName;
	private int unreadMessages;

	private List<Message> messages = new ArrayList<Message>();

	/**
	 * Constructor.
	 * 
	 * @param name The name of the folder.
	 */
	public Folder(String name) {
		this.name = name;
	}

	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getFullName() {
		return this.fullName;
	}

	public void setFullName(String fullName) {
		this.fullName = fullName;
	}

	public int getUnreadMessages() {
		return this.unreadMessages;
	}

	public void setUnreadMessages(int unreadMessages) {
		this.unreadMessages = unreadMessages;
	}

	public List<Message> getMessages() {
		return this.messages;
	}

	public void addMessage(Message message) {
		this.messages.add(message);
	}

	public void removeMessage(Message message) {
		this.messages.remove(message);
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj == null || !(obj instanceof Folder)) {
			return false;
		}

		Folder folder = (Folder) obj;

		return StringUtils.equals(folder.getFullName(), this.getFullName());
	}

	@Override
	public int hashCode() {
		return this.fullName != null ? this.fullName.hashCode() : 0;
	}
}
