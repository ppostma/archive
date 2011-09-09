package nl.pointless.webmail.web.event;

import nl.pointless.webmail.dto.Folder;

/**
 * Folder refresh action event.
 * 
 * @author Peter Postma
 */
public class FolderRefreshActionEvent {

	private final Folder folder;

	/**
	 * Constructor.
	 * 
	 * @param folder The folder to refresh.
	 */
	public FolderRefreshActionEvent(Folder folder) {
		this.folder = folder;
	}

	/**
	 * @return the folder to refresh
	 */
	public Folder getFolder() {
		return this.folder;
	}
}
