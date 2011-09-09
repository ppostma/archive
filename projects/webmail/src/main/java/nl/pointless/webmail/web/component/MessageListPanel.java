package nl.pointless.webmail.web.component;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

import nl.pointless.commons.web.behavior.FocusBehavior;
import nl.pointless.commons.web.component.AbstractLinkPropertyColumn;
import nl.pointless.commons.web.component.DatePropertyColumn;
import nl.pointless.commons.web.component.SearchTextField;
import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.event.FolderRefreshActionEvent;
import nl.pointless.webmail.web.event.FolderSelectedEvent;
import nl.pointless.webmail.web.event.MessageComposeEvent;
import nl.pointless.webmail.web.event.MessageSelectedEvent;

import org.apache.commons.lang.StringUtils;
import org.apache.wicket.ajax.AjaxRequestTarget;
import org.apache.wicket.ajax.markup.html.form.AjaxButton;
import org.apache.wicket.event.Broadcast;
import org.apache.wicket.event.IEvent;
import org.apache.wicket.extensions.markup.html.repeater.data.grid.ICellPopulator;
import org.apache.wicket.extensions.markup.html.repeater.data.table.IColumn;
import org.apache.wicket.extensions.markup.html.repeater.data.table.PropertyColumn;
import org.apache.wicket.markup.ComponentTag;
import org.apache.wicket.markup.html.WebMarkupContainer;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.markup.repeater.Item;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel that shows all messages in the selected folder.
 * 
 * @author Peter Postma
 */
public class MessageListPanel extends AbstractSwitchablePanel {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IMailService mailService;

	private MessageListDataProvider messageDataProvider;

	private IModel<Folder> folderModel;
	private IModel<Message> messageModel;

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
			}
		});

		columns.add(new PropertyColumn<Message>(new ResourceModel(
				"label.sender"), "sender"));
		columns.add(new DatePropertyColumn<Message>(new ResourceModel(
				"label.date"), "date", new SimpleDateFormat("dd-MM-yyyy")));

		MessageListDataTable dataTable = new MessageListDataTable(
				"messagesDataTableId", this.folderModel, columns,
				this.messageDataProvider, 20);
		dataTable.setOutputMarkupId(true);
		add(dataTable);

		WebMarkupContainer searchDialog = createSearchDialog(dataTable);
		add(searchDialog);
	}

	private WebMarkupContainer createSearchDialog(
			final MessageListDataTable dataTable) {
		Form<MessageListDataProvider> form = new Form<MessageListDataProvider>(
				"searchFormId");

		Label searchLabel = new Label("searchLabelId", new ResourceModel(
				"label.search"));
		form.add(searchLabel);

		SearchTextField searchTextField = new SearchTextField(
				"searchTextFieldId", new PropertyModel<String>(
						this.messageDataProvider, "filter"));
		searchTextField.add(new FocusBehavior());
		form.add(searchTextField);

		final AjaxButton searchSubmitButton = new AjaxButton("searchSubmitId",
				new ResourceModel("button.search")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onSubmit(AjaxRequestTarget target, Form<?> form) {
				target.add(dataTable);
			}

			@Override
			protected void onError(AjaxRequestTarget target, Form<?> form) {
				// Ignored
			}
		};
		form.add(searchSubmitButton);

		WebMarkupContainer searchDialog = new WebMarkupContainer(
				"searchDialogId") {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onComponentTag(ComponentTag tag) {
				super.onComponentTag(tag);

				Folder currentFolder = getCurrentFolder();
				tag.put("title", "Search in " + currentFolder.getName());
			}
		};
		searchDialog.add(form);

		return searchDialog;
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

		send(getPage(), Broadcast.EXACT, new MessageSelectedEvent(
				selectedMessage));
	}

	/**
	 * @return the current selected folder
	 */
	Folder getCurrentFolder() {
		return this.folderModel.getObject();
	}

	/**
	 * @return the current selected message
	 */
	Message getSelectedMessage() {
		return this.messageModel.getObject();
	}

	@Override
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new BasicActionButton("refreshButtonId",
				"images/refresh.png", new ResourceModel("label.refresh")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				send(getPage(), Broadcast.DEPTH, new FolderRefreshActionEvent(
						getCurrentFolder()));
			}
		});

		fragment.add(new BasicActionButton("writeButtonId", "images/write.png",
				new ResourceModel("label.write")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				send(getPage(), Broadcast.EXACT, new MessageComposeEvent(
						new Message()));
			}
		});

		fragment.add(new BasicActionButton("searchButtonId",
				"images/search.png", new ResourceModel("label.search")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// Handled by JavaScript, never called.
			}
		});

		return fragment;
	}

	@Override
	public void onEvent(IEvent<?> event) {
		if (event.getPayload() instanceof FolderSelectedEvent) {
			FolderSelectedEvent folderSelectedEvent = (FolderSelectedEvent) event
					.getPayload();

			Folder folder = folderSelectedEvent.getFolder();

			String filter = this.messageDataProvider.getFilter();

			List<Message> messages = folder.getMessages();
			this.messageDataProvider.setMessages(messages);

			if (folderSelectedEvent.isKeepFilter()
					&& StringUtils.isNotEmpty(filter)) {
				this.messageDataProvider.setFilter(filter);
			}
		}
	}
}
