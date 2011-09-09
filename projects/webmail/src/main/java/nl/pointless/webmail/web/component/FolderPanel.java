package nl.pointless.webmail.web.component;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import nl.pointless.commons.web.component.AbstractIndicatingAjaxLink;
import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.event.FolderRefreshActionEvent;
import nl.pointless.webmail.web.event.FolderSelectedEvent;

import org.apache.wicket.AttributeModifier;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.ajax.markup.html.AjaxLink;
import org.apache.wicket.event.Broadcast;
import org.apache.wicket.event.IEvent;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.list.ListItem;
import org.apache.wicket.markup.html.list.ListView;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.util.ListModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel that shows all folders in the users mailbox.
 * 
 * @author Peter Postma
 */
public class FolderPanel extends Panel {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IMailService mailService;

	private IModel<Folder> folderModel;
	private IModel<List<Folder>> folderListModel;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param folderModel Folder model.
	 */
	public FolderPanel(String id, IModel<Folder> folderModel) {
		super(id);
		this.folderModel = folderModel;
		this.folderListModel = new ListModel<Folder>() {

			private static final long serialVersionUID = 1L;

			@Override
			public List<Folder> getObject() {
				// Lazy load the folder list
				List<Folder> folders = super.getObject();
				if (folders == null) {
					refreshFolderList();

					folders = super.getObject();
				}
				return folders;
			};
		};

		ListView<Folder> folders = new ListView<Folder>("foldersId",
				this.folderListModel) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void populateItem(ListItem<Folder> item) {
				final Folder folder = item.getModelObject();
				final int unreadMessages = folder.getUnreadMessages();

				AjaxLink<Folder> folderNameLink = new AbstractIndicatingAjaxLink<Folder>(
						"folderLinkId") {

					private static final long serialVersionUID = 1L;

					@Override
					public void onClick(AjaxRequestTarget target) {
						FolderPanel.this.selectFolder(folder);

						setResponsePage(getPage());
					}
				};
				Label folderNameLabel = new Label("folderNameId",
						folder.getName());
				if (unreadMessages > 0) {
					folderNameLabel.add(AttributeModifier.replace("class",
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
	void selectFolder(Folder folder) {
		// Get the fully initialized selected folder.
		Folder currentFolder = this.mailService.getFolderByName(folder
				.getFullName());
		this.folderModel.setObject(currentFolder);

		List<Folder> folderList = this.folderListModel.getObject();

		// Replace the folder with the fully initialized one.
		int index = folderList.indexOf(folder);

		folderList.remove(index);
		folderList.add(index, currentFolder);

		send(getPage(), Broadcast.DEPTH, new FolderSelectedEvent(currentFolder));
	}

	/**
	 * Refreshes the list with folders in the users' mailbox and refresh the
	 * first or current folder.
	 */
	void refreshFolderList() {
		List<Folder> folderList = this.mailService.getFolders();
		this.folderListModel.setObject(folderList);

		// Select the first folder if none is selected, or else refresh current.
		if (!folderList.isEmpty()) {
			Folder folderToSelect;

			Folder currentFolder = this.folderModel.getObject();
			if (currentFolder == null) {
				folderToSelect = folderList.get(0);
			} else {
				folderToSelect = currentFolder;
			}

			selectFolder(folderToSelect);
		}

		// Sort the folders, standard mailbox first, then others.
		Collections.sort(folderList, new Comparator<Folder>() {

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
	Folder getCurrentFolder() {
		return this.folderModel.getObject();
	}

	@Override
	public void onEvent(IEvent<?> event) {
		if (event.getPayload() instanceof FolderRefreshActionEvent) {
			refreshFolderList();
		}
	}
}
