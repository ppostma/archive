package nl.pointless.webmail.web.event;

import nl.pointless.webmail.dto.Folder;

/**
 * Folder selected event.
 * 
 * @author Peter Postma
 */
public class FolderSelectedEvent {

	private final Folder folder;
	private final boolean keepFilter;

	/**
	 * Constructor.
	 * 
	 * @param folder The selected folder.
	 */
	public FolderSelectedEvent(Folder folder) {
		this.folder = folder;
		this.keepFilter = false;
	}

	/**
	 * Constructor.
	 * 
	 * @param folder The selected folder.
	 * @param keepFilter Keep current filter when refreshing contents?
	 */
	public FolderSelectedEvent(Folder folder, boolean keepFilter) {
		this.folder = folder;
		this.keepFilter = keepFilter;
	}

	/**
	 * @return the selected folder
	 */
	public Folder getFolder() {
		return this.folder;
	}

	/**
	 * @return if <code>true</code> then apply current filter after refreshing
	 * folder contents
	 */
	public boolean isKeepFilter() {
		return this.keepFilter;
	}
}
