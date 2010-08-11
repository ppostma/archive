package nl.pointless.webmail.web;

import nl.pointless.webmail.dto.Folder;

/**
 * Listener for folder selection events.
 * 
 * @author Peter Postma
 */
public interface IFolderSelectListener {

	/**
	 * Event fired when a folder is selected.
	 * 
	 * @param folder The new selected folder.
	 */
	void onFolderSelect(Folder folder);
}
