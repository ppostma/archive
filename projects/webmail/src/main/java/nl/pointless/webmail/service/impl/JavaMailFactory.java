package nl.pointless.webmail.service.impl;

import java.io.IOException;
import java.util.Date;

import javax.mail.MessagingException;
import javax.mail.Flags.Flag;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.parser.MessageContentParser;

/**
 * Factory class to construct lightweight transfer objects from javax.mail
 * objects.
 * 
 * @author Peter Postma
 */
public final class JavaMailFactory {

	/**
	 * Construct a {@link Folder} object from {@link javax.mail.Folder}.
	 * 
	 * @param folder the folder from the JavaMail API.
	 * @return folder
	 * @throws MessagingException
	 */
	public static Folder createFolder(javax.mail.Folder folder)
			throws MessagingException {
		Folder newFolder = new Folder(folder.getName());
		newFolder.setFullName(folder.getFullName());
		newFolder.setUnreadMessages(folder.getUnreadMessageCount());

		return newFolder;
	}

	/**
	 * Construct a {@link Message} object from {@link javax.mail.Message}. Does
	 * not initialize the body of the message.
	 * 
	 * @param message the message from the JavaMail API.
	 * @return message
	 * @throws MessagingException
	 */
	public static Message createMessage(javax.mail.Message message)
			throws MessagingException {
		Message newMessage = new Message();
		newMessage.setId(String.valueOf(message.getMessageNumber()));
		newMessage.setFolderName(message.getFolder().getFullName());
		if (message.getSubject() == null) {
			newMessage.setSubject("[no subject]");
		} else {
			newMessage.setSubject(message.getSubject());
		}
		if (message.getSentDate() == null) {
			newMessage.setDate(new Date(0));
		} else {
			newMessage.setDate(message.getSentDate());
		}
		newMessage.setRead(message.isSet(Flag.SEEN));

		javax.mail.Address[] senders = message.getFrom();
		if (senders != null) {
			StringBuilder parsedSenders = new StringBuilder();

			for (javax.mail.Address sender : senders) {
				if (parsedSenders.length() > 0) {
					parsedSenders.append(", ");
				}
				parsedSenders.append(sender.toString());
			}

			if (parsedSenders.toString() == null) {
				newMessage.setSender("[unknown sender]");
			} else {
				newMessage.setSender(parsedSenders.toString());
			}
		}

		javax.mail.Address[] recipients = message.getAllRecipients();
		if (recipients != null) {
			StringBuilder parsedRecipients = new StringBuilder();

			for (javax.mail.Address recipient : recipients) {
				if (parsedRecipients.length() > 0) {
					parsedRecipients.append(", ");
				}
				parsedRecipients.append(recipient.toString());
			}

			if (parsedRecipients.toString() == null) {
				newMessage.setRecipient("[unknown recipient]");
			} else {
				newMessage.setRecipient(parsedRecipients.toString());
			}
		}

		return newMessage;
	}

	/**
	 * Construct a {@link Message} object from {@link javax.mail.Message}.
	 * 
	 * @param message the message from the JavaMail API.
	 * @return message
	 * @throws MessagingException
	 * @throws IOException
	 */
	public static Message createMessageWithBody(javax.mail.Message message)
			throws MessagingException, IOException {
		Message newMessage = createMessage(message);

		MessageContentParser parser = new MessageContentParser(message);
		parser.parse(newMessage);

		return newMessage;
	}
}
