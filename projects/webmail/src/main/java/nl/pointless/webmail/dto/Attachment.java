package nl.pointless.webmail.dto;

import java.io.Serializable;

/**
 * An attachment in a message.
 * 
 * @author Peter Postma
 */
public class Attachment implements Serializable {

	private static final long serialVersionUID = -5105223611880089514L;

	private String filename;
	private String contentType;
	private byte[] content;

	public String getFilename() {
		return this.filename;
	}

	public void setFilename(String filename) {
		this.filename = filename;
	}

	public String getContentType() {
		return this.contentType;
	}

	public void setContentType(String contentType) {
		this.contentType = contentType;
	}

	public byte[] getContent() {
		return this.content;
	}

	public void setContent(byte[] content) {
		this.content = content;
	}
}
