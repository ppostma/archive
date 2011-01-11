package nl.pointless.webmail.service.impl;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;

/**
 * Test implementation of {@link IMailService}.
 * 
 * @author Peter Postma
 */
public class MockMailService implements IMailService {

	/**
	 * {@inheritDoc}
	 */
	public String getServiceName() {
		return "Mock";
	}

	/**
	 * {@inheritDoc}
	 */
	public Folder getFolderByName(String folderName) {
		Folder folder = new Folder(folderName);
		folder.setFullName(folderName);
		folder.addMessage(getMessageById(folderName, "1"));
		return folder;
	}

	/**
	 * {@inheritDoc}
	 */
	public List<Folder> getFolders() {
		List<Folder> folders = new ArrayList<Folder>();
		folders.add(getFolderByName("Mock"));
		return folders;
	}

	/**
	 * {@inheritDoc}
	 */
	public Message getMessageById(String folderName, String messageId) {
		Message message = new Message();
		message.setFolderName(folderName);
		message.setId(messageId);
		message.setDate(new Date());
		return message;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean markMessageRead(Message message) {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean markMessageJunk(Message message) {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public boolean deleteMessage(Message message) {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public void sendMessage(Message message) {
		// No implementation
	}
}
