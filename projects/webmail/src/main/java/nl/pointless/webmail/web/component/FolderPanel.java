package nl.pointless.webmail.web.component;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.PanelSwitcher;

import org.apache.wicket.behavior.SimpleAttributeModifier;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.link.Link;
import org.apache.wicket.markup.html.list.ListItem;
import org.apache.wicket.markup.html.list.ListView;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel that shows all folders in the users mailbox.
 * 
 * @author Peter Postma
 */
public class FolderPanel extends AbstractWebmailPanel {

	private static final long serialVersionUID = 4628129714174459843L;

	@SpringBean
	private IMailService mailService;

	private Folder currentFolder;
	private List<Folder> folderList;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param panelSwitcher A {@link PanelSwitcher}.
	 */
	public FolderPanel(String id, PanelSwitcher panelSwitcher) {
		super(id, panelSwitcher);

		ListView<Folder> folders = new ListView<Folder>("foldersId",
				new PropertyModel<List<Folder>>(this, "folderList")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void populateItem(ListItem<Folder> item) {
				final Folder folder = item.getModelObject();
				final int unreadMessages = folder.getUnreadMessages();

				Link<Folder> folderNameLink = new Link<Folder>("folderLinkId") {

					private static final long serialVersionUID = 1L;

					@Override
					public void onClick() {
						FolderPanel.this.selectFolder(folder);

						activateMessageListPanel();
					}
				};
				Label folderNameLabel = new Label("folderNameId", folder
						.getName());
				if (unreadMessages > 0) {
					folderNameLabel.add(new SimpleAttributeModifier("class",
							"unread"));
				}
				folderNameLink.add(folderNameLabel);

				Label messagesLabel = new Label("messagesId", "("
						+ unreadMessages + ")");
				messagesLabel.setVisible(unreadMessages > 0);

				item.add(folderNameLink, messagesLabel);
			}
		};

		add(folders);
	}

	/**
	 * Select the new current folder.
	 * 
	 * @param folder new selected folder
	 */
	protected void selectFolder(Folder folder) {
		// Get the fully initialized selected folder.
		this.currentFolder = this.mailService.getFolderByName(folder
				.getFullName());

		// Replace the folder with the fully initialized one.
		int index = this.folderList.indexOf(folder);

		this.folderList.remove(index);
		this.folderList.add(index, this.currentFolder);
	}

	/**
	 * Refreshes the list with folders in the users' mailbox and refresh the
	 * first or current folder.
	 */
	public void refreshFolderList() {
		this.folderList = this.mailService.getFolders();

		// Select the first folder if none is selected, or else refresh current.
		if (!this.folderList.isEmpty()) {
			Folder folderToSelect;

			if (this.currentFolder == null) {
				folderToSelect = this.folderList.get(0);
			} else {
				folderToSelect = this.currentFolder;
			}

			selectFolder(folderToSelect);
		}

		// Sort the folders, standard mailbox first, then others.
		Collections.sort(this.folderList, new Comparator<Folder>() {

			public int compare(Folder f1, Folder f2) {
				if (f1.getName().equals("INBOX")) {
					return -1;
				}

				return f1.getName().compareTo(f2.getName());
			}
		});
	}

	/**
	 * @return the current selected folder
	 */
	public Folder getCurrentFolder() {
		return this.currentFolder;
	}

	/**
	 * @return the current list with folders
	 */
	public List<Folder> getFolderList() {
		if (this.folderList == null) {
			refreshFolderList();
		}

		return this.folderList;
	}
}
