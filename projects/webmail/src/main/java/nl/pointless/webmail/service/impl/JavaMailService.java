package nl.pointless.webmail.service.impl;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.mail.Flags;
import javax.mail.MessagingException;
import javax.mail.NoSuchProviderException;
import javax.mail.URLName;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;

import org.apache.log4j.Logger;

/**
 * Implementation of {@link IMailService}. Uses the JavaMail API to retrieve
 * mail.
 * 
 * @author Peter Postma
 */
public class JavaMailService implements IMailService {

	private static final Logger log = Logger.getLogger(JavaMailService.class);

	private static final String JUNK_FOLDER = "INBOX.Spam";
	private static final String TRASH_FOLDER = "INBOX.Trash";

	private JavaMailSession mailSession;

	/**
	 * {@inheritDoc}
	 */
	public String getServiceName() {
		String name = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			URLName urlName = store.getURLName();
			name = urlName.toString();

		} catch (NoSuchProviderException e) {
			log.error(e.getMessage(), e);
		}

		return name;
	}

	/**
	 * {@inheritDoc}
	 */
	public List<Folder> getFolders() {
		List<Folder> newFolders = new ArrayList<Folder>();

		try {
			javax.mail.Store store = this.mailSession.getStore();
			javax.mail.Folder[] folders = store.getPersonalNamespaces();

			for (javax.mail.Folder folder : folders) {
				Folder newFolder = JavaMailFactory.createFolder(folder);

				newFolders.add(newFolder);

				for (javax.mail.Folder subFolder : folder.list()) {
					Folder newSubFolder = JavaMailFactory
							.createFolder(subFolder);

					newFolders.add(newSubFolder);
				}
			}
		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		}

		return newFolders;
	}

	/**
	 * {@inheritDoc}
	 */
	public Folder getFolderByName(String folderName) {
		Folder newFolder = null;
		javax.mail.Folder folder = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			folder = store.getFolder(folderName);
			folder.open(javax.mail.Folder.READ_ONLY);

			newFolder = JavaMailFactory.createFolder(folder);

			javax.mail.Message[] messages = folder.getMessages();
			for (javax.mail.Message message : messages) {
				// Skip deleted messages.
				if (message.isSet(Flags.Flag.DELETED)) {
					continue;
				}

				Message newMessage = JavaMailFactory.createMessage(message);

				newFolder.addMessage(newMessage);
			}

		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		} finally {
			try {
				if (folder != null) {
					folder.close(false);
				}
			} catch (MessagingException e) {
				log.warn(e.getMessage(), e);
			}
		}

		return newFolder;
	}

	/**
	 * {@inheritDoc}
	 */
	public Message getMessageById(String folderName, String messageId) {
		Message newMessage = null;
		javax.mail.Folder folder = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			folder = store.getFolder(folderName);
			folder.open(javax.mail.Folder.READ_ONLY);

			int msgnum = Integer.parseInt(messageId);
			javax.mail.Message message = folder.getMessage(msgnum);

			newMessage = JavaMailFactory.createMessageWithBody(message);

		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		} catch (IOException e) {
			log.error(e.getMessage(), e);
		} catch (NumberFormatException e) {
			log.error(e.getMessage(), e);
		} finally {
			try {
				if (folder != null) {
					folder.close(false);
				}
			} catch (MessagingException e) {
				log.warn(e.getMessage(), e);
			}
		}

		return newMessage;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean markMessageRead(Message message) {
		boolean changed = false;
		javax.mail.Folder folder = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			folder = store.getFolder(message.getFolderName());
			folder.open(javax.mail.Folder.READ_WRITE);

			int msgnum = Integer.parseInt(message.getId());
			javax.mail.Message foundMessage = folder.getMessage(msgnum);

			if (!foundMessage.isSet(Flags.Flag.SEEN)) {
				foundMessage.setFlag(Flags.Flag.SEEN, true);
				changed = true;
			}

		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		} finally {
			try {
				if (folder != null) {
					folder.close(false);
				}
			} catch (MessagingException e) {
				log.warn(e.getMessage(), e);
			}
		}

		return changed;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean markMessageJunk(Message message) {
		boolean changed = false;

		javax.mail.Folder folder = null;
		javax.mail.Folder junkFolder = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			folder = store.getFolder(message.getFolderName());
			folder.open(javax.mail.Folder.READ_WRITE);

			// Move the message to the junk folder if the open folder isn't the
			// junk folder itself.
			if (!folder.getFullName().equals(JavaMailService.JUNK_FOLDER)) {
				int msgnum = Integer.parseInt(message.getId());
				javax.mail.Message foundMessage = folder.getMessage(msgnum);

				junkFolder = store.getFolder(JavaMailService.JUNK_FOLDER);
				junkFolder.open(javax.mail.Folder.READ_WRITE);

				folder.copyMessages(new javax.mail.Message[] { foundMessage },
						junkFolder);

				if (!foundMessage.isSet(Flags.Flag.DELETED)) {
					foundMessage.setFlag(Flags.Flag.DELETED, true);
					changed = true;
				}
			}

		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		} finally {
			try {
				if (junkFolder != null) {
					junkFolder.close(false);
				}
				if (folder != null) {
					folder.close(false);
				}
			} catch (MessagingException e) {
				log.warn(e.getMessage(), e);
			}
		}

		return changed;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean deleteMessage(Message message) {
		boolean changed = false;

		javax.mail.Folder folder = null;
		javax.mail.Folder trashFolder = null;

		try {
			javax.mail.Store store = this.mailSession.getStore();

			folder = store.getFolder(message.getFolderName());
			folder.open(javax.mail.Folder.READ_WRITE);

			int msgnum = Integer.parseInt(message.getId());
			javax.mail.Message foundMessage = folder.getMessage(msgnum);

			// Copy the message to the trash folder if the open folder isn't the
			// trash folder itself.
			if (!folder.getFullName().equals(JavaMailService.TRASH_FOLDER)) {
				trashFolder = store.getFolder(JavaMailService.TRASH_FOLDER);
				trashFolder.open(javax.mail.Folder.READ_WRITE);

				folder.copyMessages(new javax.mail.Message[] { foundMessage },
						trashFolder);
			}

			if (!foundMessage.isSet(Flags.Flag.DELETED)) {
				foundMessage.setFlag(Flags.Flag.DELETED, true);
				changed = true;
			}

		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		} finally {
			try {
				if (trashFolder != null) {
					trashFolder.close(false);
				}
				if (folder != null) {
					folder.close(false);
				}
			} catch (MessagingException e) {
				log.warn(e.getMessage(), e);
			}
		}

		return changed;
	}

	/**
	 * {@inheritDoc}
	 */
	public void sendMessage(Message message) {
		// TODO Auto-generated method stub

	}

	public void setMailSession(JavaMailSession mailSession) {
		this.mailSession = mailSession;
	}
}
