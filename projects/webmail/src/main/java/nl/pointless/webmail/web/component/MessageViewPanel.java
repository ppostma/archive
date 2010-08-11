package nl.pointless.webmail.web.component;

import static nl.pointless.webmail.web.component.WebmailPage.MESSAGE_WRITE_PANEL_ID;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import nl.pointless.webmail.dto.Attachment;
import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.commons.io.FileUtils;
import org.apache.wicket.RequestCycle;
import org.apache.wicket.datetime.markup.html.basic.DateLabel;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.basic.MultiLineLabel;
import org.apache.wicket.markup.html.link.Link;
import org.apache.wicket.markup.html.list.ListItem;
import org.apache.wicket.markup.html.list.ListView;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.model.AbstractReadOnlyModel;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.request.target.resource.ResourceStreamRequestTarget;
import org.apache.wicket.spring.injection.annot.SpringBean;
import org.apache.wicket.util.resource.IResourceStream;
import org.apache.wicket.util.resource.ResourceStreamNotFoundException;
import org.apache.wicket.util.time.Time;

/**
 * Panel for viewing the contents of a message, including attachments.
 * 
 * @author Peter Postma
 */
public class MessageViewPanel extends AbstractSwitchablePanel {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IMailService mailService;

	private IModel<Folder> folderModel;
	private IModel<Message> messageModel;

	/**
	 * Implementation of {@link IResourceStream} for message attachments.
	 * 
	 * @author Peter Postma
	 */
	private class AttachmentResourceStream implements IResourceStream {

		private static final long serialVersionUID = 1L;

		private Attachment attachment;
		private InputStream inputStream;
		private Locale locale;

		/**
		 * Constructor.
		 * 
		 * @param attachment The attachment to use as resource.
		 */
		public AttachmentResourceStream(Attachment attachment) {
			this.attachment = attachment;
		}

		/**
		 * {@inheritDoc}
		 */
		public void close() throws IOException {
			if (this.inputStream != null) {
				this.inputStream.close();
			}
		}

		/**
		 * {@inheritDoc}
		 */
		public String getContentType() {
			return this.attachment.getContentType();
		}

		/**
		 * {@inheritDoc}
		 */
		public InputStream getInputStream()
				throws ResourceStreamNotFoundException {
			if (this.inputStream == null) {
				this.inputStream = new ByteArrayInputStream(
						this.attachment.getContent());
			}
			return this.inputStream;
		}

		/**
		 * {@inheritDoc}
		 */
		public Locale getLocale() {
			return this.locale;
		}

		/**
		 * {@inheritDoc}
		 */
		public long length() {
			return this.attachment.getContent().length;
		}

		/**
		 * {@inheritDoc}
		 */
		public void setLocale(Locale locale) {
			this.locale = locale;
		}

		/**
		 * {@inheritDoc}
		 */
		public Time lastModifiedTime() {
			return Time.now();
		}
	}

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param folderModel Model with the current folder.
	 * @param messageModel Model with the message to show.
	 */
	public MessageViewPanel(String id, IModel<Folder> folderModel,
			IModel<Message> messageModel) {
		super(id);
		this.folderModel = folderModel;
		this.messageModel = messageModel;

		add(new Label("subjectLabelId", new ResourceModel("label.subject")));
		add(new Label("subjectId", new PropertyModel<String>(messageModel,
				"subject")));

		add(new Label("fromLabelId", new ResourceModel("label.sender")));
		add(new Label("fromId", new PropertyModel<String>(messageModel,
				"sender")));

		add(new Label("toLabelId", new ResourceModel("label.recipient")));
		add(new Label("toId", new PropertyModel<String>(messageModel,
				"recipient")));

		add(new Label("dateLabelId", new ResourceModel("label.date")));
		add(DateLabel.forDatePattern("dateId", new PropertyModel<Date>(
				messageModel, "date"), "dd MMM yyyy HH:mm"));

		add(new MultiLineLabel("bodyId", new PropertyModel<String>(
				messageModel, "body")));

		ListView<Attachment> attachments = new ListView<Attachment>(
				"attachmentsId", new PropertyModel<List<Attachment>>(
						messageModel, "attachments")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void populateItem(ListItem<Attachment> item) {
				final Attachment attachment = item.getModelObject();

				Link<Attachment> attachmentLink = new Link<Attachment>(
						"attachmentLinkId") {

					private static final long serialVersionUID = 1L;

					@Override
					public void onClick() {
						AttachmentResourceStream resourceStream = new AttachmentResourceStream(
								attachment);
						ResourceStreamRequestTarget target = new ResourceStreamRequestTarget(
								resourceStream, attachment.getContentType());
						target.setFileName(attachment.getFilename());
						RequestCycle.get().setRequestTarget(target);
					}
				};
				Label attachmentLabel = new Label("attachmentLabelId",
						attachment.getFilename());
				attachmentLink.add(attachmentLabel);
				item.add(attachmentLink);

				IModel<String> bytesLabelModel = new AbstractReadOnlyModel<String>() {

					private static final long serialVersionUID = 1L;

					@Override
					public String getObject() {
						int size = attachment.getContent().length;

						StringBuilder sb = new StringBuilder();
						sb.append("(");
						sb.append(FileUtils.byteCountToDisplaySize(size));
						sb.append(")");
						return sb.toString();
					}
				};
				Label bytesLabel = new Label("bytesLabelId", bytesLabelModel);
				item.add(bytesLabel);
			}

			@Override
			public boolean isVisible() {
				return !getModelObject().isEmpty();
			}
		};
		add(attachments);
	}

	@Override
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new AbstractActionButton("folderButtonId",
				"images/folderlist.png", new ResourceModel("label.folder")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				WebmailSession.get().getPanelSwitcher()
						.setActivePanelToPreviousPanel();
			}
		});

		fragment.add(new AbstractActionButton("writeButtonId",
				"images/write.png", new ResourceModel("label.write")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				WebmailSession.get().getPanelSwitcher()
						.setActivePanel(MESSAGE_WRITE_PANEL_ID);
			}
		});

		fragment.add(new AbstractActionButton("replyButtonId",
				"images/reply.png", new ResourceModel("label.reply")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		fragment.add(new AbstractActionButton("forwardButtonId",
				"images/forward.png", new ResourceModel("label.forward")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		fragment.add(new AbstractActionButton("deleteButtonId",
				"images/delete.png", new ResourceModel("label.delete")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Message message = getMessageModel().getObject();

				if (message != null) {
					// Mark message as deleted on the mail provider.
					boolean deleted = getMailService().setMessageDeleted(
							message.getFolderName(), message.getId());

					// On success, remove the message from the folder model.
					if (deleted) {
						getFolderModel().getObject().removeMessage(message);

						WebmailSession.get().getPanelSwitcher()
								.setActivePanelToPreviousPanel();
					}
				}
			}
		});

		fragment.add(new AbstractActionButton("spamButtonId",
				"images/junk.png", new ResourceModel("label.spam")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Message message = getMessageModel().getObject();

				if (message != null) {
					// Mark the message as junk on the mail provider.
					boolean success = getMailService().setMessageJunk(
							message.getFolderName(), message.getId());

					// On success, remove the message from the folder model.
					if (success) {
						getFolderModel().getObject().removeMessage(message);

						WebmailSession.get().getPanelSwitcher()
								.setActivePanelToPreviousPanel();
					}
				}
			}
		});

		fragment.add(new AbstractActionButton("previousButtonId",
				"images/previous.png", new ResourceModel("label.previous")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		fragment.add(new AbstractActionButton("nextButtonId",
				"images/next.png", new ResourceModel("label.next")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		return fragment;
	}

	/**
	 * @return the mail service.
	 */
	protected IMailService getMailService() {
		return this.mailService;
	}

	/**
	 * @return the folder model.
	 */
	protected IModel<Folder> getFolderModel() {
		return this.folderModel;
	}

	/**
	 * @return the message model.
	 */
	protected IModel<Message> getMessageModel() {
		return this.messageModel;
	}
}
