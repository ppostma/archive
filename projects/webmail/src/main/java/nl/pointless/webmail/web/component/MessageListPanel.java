package nl.pointless.webmail.web.component;

import static nl.pointless.webmail.web.component.WebmailPage.MESSAGE_VIEW_PANEL_ID;
import static nl.pointless.webmail.web.component.WebmailPage.MESSAGE_WRITE_PANEL_ID;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

import nl.pointless.commons.web.component.AbstractLinkPropertyColumn;
import nl.pointless.commons.web.component.DatePropertyColumn;
import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.IFolderRefreshActionListener;
import nl.pointless.webmail.web.IFolderSelectListener;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.Page;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.extensions.ajax.markup.html.modal.ModalWindow;
import org.apache.wicket.extensions.ajax.markup.html.modal.ModalWindow.MaskType;
import org.apache.wicket.extensions.markup.html.repeater.data.grid.ICellPopulator;
import org.apache.wicket.extensions.markup.html.repeater.data.table.IColumn;
import org.apache.wicket.extensions.markup.html.repeater.data.table.PropertyColumn;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.markup.repeater.Item;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel that shows all messages in the selected folder.
 * 
 * @author Peter Postma
 */
public class MessageListPanel extends AbstractSwitchablePanel implements
		IFolderSelectListener {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IMailService mailService;

	private MessageListDataProvider messageDataProvider;

	private IModel<Folder> folderModel;
	private IModel<Message> messageModel;

	private IFolderRefreshActionListener folderRefreshActionListener;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param messageModel Model for the selected message.
	 * @param folderModel Model for the selected folder.
	 */
	public MessageListPanel(String id, IModel<Folder> folderModel,
			IModel<Message> messageModel) {
		super(id);
		this.folderModel = folderModel;
		this.messageModel = messageModel;
		this.messageDataProvider = new MessageListDataProvider();

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
						.setActivePanel(MESSAGE_VIEW_PANEL_ID);
			}
		});

		columns.add(new PropertyColumn<Message>(new ResourceModel(
				"label.sender"), "sender"));
		columns.add(new DatePropertyColumn<Message>(new ResourceModel(
				"label.date"), "date", new SimpleDateFormat("dd-MM-yyyy")));

		MessageListDataTable dataTable = new MessageListDataTable(
				"messagesDataTableId", this.folderModel, columns,
				this.messageDataProvider, 25);
		dataTable.setOutputMarkupId(true);
		add(dataTable);

		ModalWindow searchWindow = createModalWindow(dataTable);
		add(searchWindow);
	}

	private ModalWindow createModalWindow(final MessageListDataTable dataTable) {
		ModalWindow searchWindow = new ModalWindow("searchDialogId");
		searchWindow.setMinimalHeight(30);
		searchWindow.setInitialHeight(30);
		searchWindow.setMinimalWidth(300);
		searchWindow.setInitialWidth(300);
		searchWindow.setMaskType(MaskType.TRANSPARENT);
		searchWindow.setCssClassName(ModalWindow.CSS_CLASS_GRAY);
		searchWindow.setPageCreator(new ModalWindow.PageCreator() {

			private static final long serialVersionUID = 1L;

			/**
			 * {@inheritDoc}
			 */
			public Page createPage() {
				return new SearchDialogPage(getSearchWindow(),
						getMessageDataProvider());
			}
		});
		searchWindow
				.setWindowClosedCallback(new ModalWindow.WindowClosedCallback() {

					private static final long serialVersionUID = 1L;

					/**
					 * {@inheritDoc}
					 */
					public void onClose(AjaxRequestTarget target) {
						target.addComponent(dataTable);
					}
				});
		return searchWindow;
	}

	/**
	 * Mark the message as read and select the new message.
	 * 
	 * @param message selected message.
	 */
	void selectMessage(Message message) {
		// Mark the message as read on the mail provider.
		boolean result = this.mailService.markMessageRead(message);

		// On success, mark the message as read on the model objects.
		if (result) {
			Folder folder = this.folderModel.getObject();
			folder.setUnreadMessages(folder.getUnreadMessages() - 1);

			message.setRead(true);
		}

		// Get the fully initialized selected message.
		Message selectedMessage = this.mailService.getMessageById(
				message.getFolderName(), message.getId());
		this.messageModel.setObject(selectedMessage);
	}

	/**
	 * @return the message list data provider.
	 */
	MessageListDataProvider getMessageDataProvider() {
		return this.messageDataProvider;
	}

	/**
	 * @return the modal window
	 */
	ModalWindow getSearchWindow() {
		return (ModalWindow) get("searchDialogId");
	}

	/**
	 * @return the folder refresh action listener
	 */
	IFolderRefreshActionListener getFolderRefreshActionListener() {
		return this.folderRefreshActionListener;
	}

	/**
	 * Add a listener for folder refresh actions.
	 * 
	 * @param folderRefreshActionListener A {@link IFolderRefreshActionListener}
	 * .
	 */
	void addFolderRefreshActionListener(
			IFolderRefreshActionListener folderRefreshActionListener) {
		this.folderRefreshActionListener = folderRefreshActionListener;
	}

	@Override
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new BasicActionButton("refreshButtonId",
				"images/refresh.png", new ResourceModel("label.refresh")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				getFolderRefreshActionListener().onActionRefreshFolders();
			}
		});

		fragment.add(new BasicActionButton("writeButtonId",
				"images/write.png", new ResourceModel("label.write")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				WebmailSession.get().getPanelSwitcher()
						.setActivePanel(MESSAGE_WRITE_PANEL_ID);
			}
		});

		fragment.add(new AjaxActionButton("searchButtonId",
				"images/search.png", new ResourceModel("label.search")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick(AjaxRequestTarget target) {
				getSearchWindow().show(target);
			}
		});

		return fragment;
	}

	/**
	 * {@inheritDoc}
	 */
	public void onFolderSelect(Folder folder) {
		List<Message> messages = folder.getMessages();
		this.messageDataProvider.setMessages(messages);
	}
}
