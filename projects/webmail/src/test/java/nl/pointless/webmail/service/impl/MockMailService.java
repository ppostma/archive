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

	public String getServiceName() {
		return "Mock";
	}

	public Folder getFolderByName(String folderName) {
		Folder folder = new Folder(folderName);
		folder.setFullName(folderName);
		folder.addMessage(getMessageById(folderName, "1"));
		return folder;
	}

	public List<Folder> getFolders() {
		List<Folder> folders = new ArrayList<Folder>();
		folders.add(getFolderByName("Mock"));
		return folders;
	}

	public Message getMessageById(String folderName, String messageId) {
		Message message = new Message();
		message.setFolderName(folderName);
		message.setId(messageId);
		message.setDate(new Date());
		return message;
	}

	public boolean setMessageRead(String folderName, String messageId) {
		return false;
	}

	public boolean setMessageDeleted(String folderName, String messageId) {
		return false;
	}

	public boolean setMessageJunk(String folderName, String messageId) {
		return false;
	}
}
