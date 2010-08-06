package nl.pointless.webmail.web.component;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Iterator;
import java.util.List;

import nl.pointless.commons.web.component.AbstractLinkPropertyColumn;
import nl.pointless.commons.web.component.DatePropertyColumn;
import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.extensions.markup.html.repeater.data.grid.ICellPopulator;
import org.apache.wicket.extensions.markup.html.repeater.data.table.IColumn;
import org.apache.wicket.extensions.markup.html.repeater.data.table.PropertyColumn;
import org.apache.wicket.extensions.markup.html.repeater.util.SortableDataProvider;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.markup.repeater.Item;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel that shows all messages in the selected folder.
 * 
 * @author Peter Postma
 */
public class MessageListPanel extends AbstractSwitchablePanel {

	private static final long serialVersionUID = -7598543531453247995L;

	public static final String PANEL_ID = "messageListId";

	private FolderPanel folderPanel;

	private IModel<Folder> folderModel;
	private Message selectedMessage;

	@SpringBean
	private IMailService mailService;

	/**
	 * SortableDataProvider for {@link Message} lists.
	 * 
	 * @author Peter Postma
	 */
	class MessageListDataProvider extends SortableDataProvider<Message> {

		private static final long serialVersionUID = 1L;

		/**
		 * Cached messages.
		 */
		private List<Message> messages;

		/**
		 * Constructor.
		 */
		public MessageListDataProvider() {
			this.setSort("date", false);
		}

		/**
		 * {@inheritDoc}
		 */
		public Iterator<Message> iterator(int first, int count) {
			List<Message> newList = new ArrayList<Message>(getMessagesCached());

			final String property = getSort().getProperty();
			final boolean ascending = getSort().isAscending();

			Collections.sort(newList, new Comparator<Message>() {

				public int compare(Message m1, Message m2) {
					PropertyModel<Object> model1 = new PropertyModel<Object>(
							m1, property);
					PropertyModel<Object> model2 = new PropertyModel<Object>(
							m2, property);

					Object modelObject1 = model1.getObject();
					Object modelObject2 = model2.getObject();

					int compare = ((Comparable) modelObject1)
							.compareTo(modelObject2);

					if (!ascending) {
						compare *= -1;
					}

					return compare;
				}
			});

			return newList.subList(first, first + count).iterator();
		}

		/**
		 * {@inheritDoc}
		 */
		public IModel<Message> model(Message message) {
			return new Model<Message>(message);
		}

		/**
		 * {@inheritDoc}
		 */
		public int size() {
			return getMessagesCached().size();
		}

		@Override
		public void detach() {
			this.messages = null;
		}

		private List<Message> getMessagesCached() {
			if (this.messages == null) {
				this.messages = getMessagesFromModel();
			}
			return this.messages;
		}
	}

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param folderPanel The folder panel.
	 * @param folderModel Model for the selected folder.
	 */
	public MessageListPanel(String id, FolderPanel folderPanel,
			IModel<Folder> folderModel) {
		super(id);
		this.folderPanel = folderPanel;
		this.folderModel = folderModel;

		List<IColumn<Message>> columns = new ArrayList<IColumn<Message>>();
		columns.add(new AbstractLinkPropertyColumn<Message>(new ResourceModel(
				"label.subject"), "subject") {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick(Item<ICellPopulator<Message>> item,
					String componentId, IModel<Message> model) {
				Message message = model.getObject();

				MessageListPanel.this.selectMessage(message);

				WebmailSession.get().getPanelSwitcher()
						.setActivePanel(MessageViewPanel.PANEL_ID);
			}
		});

		columns.add(new PropertyColumn<Message>(new ResourceModel(
				"label.sender"), "sender"));
		columns.add(new DatePropertyColumn<Message>(new ResourceModel(
				"label.date"), "date", new SimpleDateFormat("dd-MM-yyyy")));

		MessageListDataProvider messageDataProvider = new MessageListDataProvider();

		MessageListDataTable dataTable = new MessageListDataTable(
				"messagesDataTableId", this.folderModel, columns,
				messageDataProvider, 25);
		add(dataTable);
	}

	/**
	 * Returns the list with messages from the folder model. Returns an empty
	 * list if the folder is not found.
	 * 
	 * @param folderModel The model for the folder.
	 * @return list with messages from the folder model.
	 */
	protected List<Message> getMessagesFromModel() {
		Folder folder = this.folderModel.getObject();
		if (folder == null) {
			return new ArrayList<Message>();
		}
		return folder.getMessages();
	}

	/**
	 * Mark the message as read and select the new message.
	 * 
	 * @param message selected message.
	 */
	protected void selectMessage(Message message) {
		// Mark the message as read on the mail provider.
		boolean result = this.mailService.setMessageRead(
				message.getFolderName(), message.getId());

		// On success, mark the message as read on the model objects.
		if (result) {
			Folder folder = this.folderModel.getObject();
			folder.setUnreadMessages(folder.getUnreadMessages() - 1);

			message.setRead(true);
		}

		// Get the fully initialized selected message.
		this.selectedMessage = this.mailService.getMessageById(
				message.getFolderName(), message.getId());
	}

	/**
	 * @return the selected message.
	 */
	public Message getSelectedMessage() {
		return this.selectedMessage;
	}

	/**
	 * @return the folder panel.
	 */
	protected FolderPanel getFolderPanel() {
		return this.folderPanel;
	}

	/**
	 * {@inheritDoc}
	 */
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new AbstractActionButton("refreshButtonId",
				"images/refresh.png", new ResourceModel("label.refresh")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				getFolderPanel().refreshFolderList();
			}
		});

		fragment.add(new AbstractActionButton("writeButtonId",
				"images/write.png", new ResourceModel("label.write")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				WebmailSession.get().getPanelSwitcher()
						.setActivePanel(MessageWritePanel.PANEL_ID);
			}
		});

		fragment.add(new AbstractActionButton("searchButtonId",
				"images/search.png", new ResourceModel("label.search")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me!
			}
		});

		return fragment;
	}
}
