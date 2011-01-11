package nl.pointless.webmail.service;

import java.util.List;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;

/**
 * Interface with methods to retrieve and manipulate mail.
 * 
 * @author Peter Postma
 */
public interface IMailService {

	/**
	 * Returns the string representation of the service we're connected to.
	 * 
	 * @return string representation of the connected service
	 */
	String getServiceName();

	/**
	 * Returns a list with folders in the users' mailbox. All attributes of
	 * {@link Folder} are expected to be initialized except for the list with
	 * messages, see {@link #getFolderByName(String)}.
	 * 
	 * @return list with folders
	 */
	List<Folder> getFolders();

	/**
	 * Returns the folder in the users' mailbox. This method returns a fully
	 * initialized {@link Folder}, including the list with messages in the
	 * folder.
	 * 
	 * @param folderName The (full) folder name to search for.
	 * @return the folder
	 */
	Folder getFolderByName(String folderName);

	/**
	 * Retrieves the message in the requested folder, with the requested message
	 * Id.
	 * 
	 * @param folderName The (full) folder name to look in.
	 * @param messageId The message Id.
	 * @return the message
	 */
	Message getMessageById(String folderName, String messageId);

	/**
	 * Marks a message as read.
	 * 
	 * @param message The message.
	 * @return <code>true</code> if marking the message read was successful
	 */
	boolean markMessageRead(Message message);

	/**
	 * Marks a message as junk. The message will be removed from the current
	 * folder and will be placed into the Junk folder.
	 * 
	 * @param message The message.
	 * @return <code>true</code> if marking the message as junk was successful
	 */
	boolean markMessageJunk(Message message);

	/**
	 * Marks a message as deleted. The message will be removed from the current
	 * folder and will be placed into the Trash folder. If this action is
	 * invoked on a message which is already in the Trash folder, then the
	 * message will be deleted permanently.
	 * 
	 * @param message The message.
	 * @return <code>true</code> if marking the message deleted was successful
	 */
	boolean deleteMessage(Message message);

	/**
	 * Send the message.
	 * 
	 * @param message The message.
	 */
	void sendMessage(Message message);
}
