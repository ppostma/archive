package nl.pointless.webmail.parser;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.mail.BodyPart;
import javax.mail.MessagingException;
import javax.mail.internet.MimeMultipart;

import nl.pointless.webmail.dto.Attachment;
import nl.pointless.webmail.dto.Message;

import org.apache.commons.io.IOUtils;

/**
 * Parses the body (content) of a {@link javax.mail.Message}. Prefer the
 * text/plain content, but fall back to HTML if plain text isn't available.
 * 
 * @author Peter Postma
 */
public class MessageContentParser {

	private javax.mail.Message message;

	private Map<String, String> contentTypes;
	private List<Attachment> attachments;

	/**
	 * Constructor.
	 * 
	 * @param message The message to parse.
	 */
	public MessageContentParser(javax.mail.Message message) {
		this.message = message;
	}

	/**
	 * Parse the JavaMail message and fill the DTO with information we want.
	 * 
	 * @param newMessage The message DTO to put parsed information in.
	 * @throws IOException
	 * @throws MessagingException
	 */
	public void parse(Message newMessage) throws IOException,
			MessagingException {
		this.contentTypes = new HashMap<String, String>();
		this.attachments = new ArrayList<Attachment>();

		parseContent(this.message);

		String content = this.contentTypes.get("text/plain");
		if (content != null) {
			newMessage.setBody(content);
		} else {
			content = this.contentTypes.get("text/html");
			if (content != null) {
				StringBuilder newContent = new StringBuilder();
				newContent
						.append("[Note: This is the converted text/html version of the e-mail, because text/plain is not available.]");
				newContent.append("\n\n");

				String convertedContent = convertHtmlToText(content);
				newContent.append(convertedContent);

				newMessage.setBody(newContent.toString());
			} else {
				newMessage
						.setBody("[Unknown content type. The content type for this e-mail could not be parsed or is not supported.]");
			}
		}

		newMessage.setAttachments(this.attachments);
	}

	private void parseContent(javax.mail.Part messagePart) throws IOException,
			MessagingException {
		parseContent(messagePart, false);
	}

	private void parseContent(javax.mail.Part messagePart,
			boolean alternativeContent) throws IOException, MessagingException {
		Object content = messagePart.getContent();
		String contentType = messagePart.getContentType();
		boolean isAlternative = contentType.startsWith("multipart/alternative")
				|| alternativeContent;

		if (messagePart.isMimeType("multipart/*")) {
			MimeMultipart multipart = (MimeMultipart) content;

			for (int i = 0; i < multipart.getCount(); i++) {
				BodyPart bodyPart = multipart.getBodyPart(i);

				parseContent(bodyPart, isAlternative);
			}
		} else {
			parseNonMultipartContent(messagePart, isAlternative);
		}
	}

	private void parseNonMultipartContent(javax.mail.Part messagePart,
			boolean alternativeContent) throws IOException, MessagingException {
		Object content = messagePart.getContent();
		String contentType = messagePart.getContentType();
		String filename = messagePart.getFileName();

		// Strip any extra info from the MIME type.
		int index = contentType.indexOf(";");
		if (index != -1) {
			contentType = contentType.substring(0, index);
		}

		// Parse the plain text or HTML.
		if (filename == null
				&& (contentType.equals("text/plain") || contentType
						.equals("text/html"))) {
			String contentBody = this.contentTypes.get(contentType);

			if (!alternativeContent && contentBody != null) {
				// If this is not alternative content, append it.
				StringBuilder newBody = new StringBuilder();
				newBody.append(contentBody);
				newBody.append("\n");
				newBody.append(content.toString());

				this.contentTypes.put(contentType, newBody.toString());
			} else {
				this.contentTypes.put(contentType, content.toString());
			}
		}

		// Parse attachments.
		if (filename != null) {
			InputStream input = messagePart.getInputStream();
			ByteArrayOutputStream output = new ByteArrayOutputStream();
			IOUtils.copy(input, output);

			Attachment attachment = new Attachment();
			attachment.setContent(output.toByteArray());
			attachment.setContentType(contentType);
			attachment.setFilename(filename);
			this.attachments.add(attachment);
		}
	}

	private String convertHtmlToText(String content) {
		String convertedContent = content;

		// TODO Use a better parser.
		convertedContent = content.replaceAll("<.*?>", "");
		convertedContent = convertedContent.replaceAll("&nbsp;", " ");

		return convertedContent;
	}
}
