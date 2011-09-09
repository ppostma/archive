package nl.pointless.webmail.dto;

import java.io.Serializable;
import java.util.Date;
import java.util.List;

import org.apache.commons.lang.StringUtils;

/**
 * Lightweight object for a mail message.
 * 
 * @author Peter Postma
 */
public class Message implements Serializable {

	private static final long serialVersionUID = 1L;

	private String id;
	private String folderName;
	private String subject;
	private String sender;
	private String recipient;
	private String body;
	private Date date;
	private boolean read;
	private List<Attachment> attachments;

	public String getId() {
		return this.id;
	}

	public void setId(String id) {
		this.id = id;
	}

	public String getFolderName() {
		return this.folderName;
	}

	public void setFolderName(String folderName) {
		this.folderName = folderName;
	}

	public String getSubject() {
		return this.subject;
	}

	public void setSubject(String subject) {
		this.subject = subject;
	}

	public String getSender() {
		return this.sender;
	}

	public void setSender(String sender) {
		this.sender = sender;
	}

	public String getRecipient() {
		return this.recipient;
	}

	public void setRecipient(String recipient) {
		this.recipient = recipient;
	}

	public String getBody() {
		return this.body;
	}

	public void setBody(String body) {
		this.body = body;
	}

	public Date getDate() {
		return this.date;
	}

	public void setDate(Date date) {
		this.date = date;
	}

	public boolean isRead() {
		return this.read;
	}

	public void setRead(boolean read) {
		this.read = read;
	}

	public List<Attachment> getAttachments() {
		return this.attachments;
	}

	public void setAttachments(List<Attachment> attachments) {
		this.attachments = attachments;
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj == null || !(obj instanceof Message)) {
			return false;
		}

		Message message = (Message) obj;

		return StringUtils.equals(message.getId(), this.getId());
	}

	@Override
	public int hashCode() {
		return this.id != null ? this.id.hashCode() : 0;
	}
}
