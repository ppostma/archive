package nl.pointless.webmail.web.component;

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
import nl.pointless.webmail.web.event.FolderSelectedEvent;
import nl.pointless.webmail.web.event.MessageComposeEvent;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.wicket.datetime.markup.html.basic.DateLabel;
import org.apache.wicket.event.Broadcast;
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
import org.apache.wicket.request.cycle.RequestCycle;
import org.apache.wicket.request.handler.resource.ResourceStreamRequestHandler;
import org.apache.wicket.spring.injection.annot.SpringBean;
import org.apache.wicket.util.lang.Bytes;
import org.apache.wicket.util.resource.AbstractResourceStream;
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
	private class AttachmentResourceStream extends AbstractResourceStream {

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
		@Override
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
		@Override
		public Locale getLocale() {
			return this.locale;
		}

		/**
		 * {@inheritDoc}
		 */
		@Override
		public Bytes length() {
			return Bytes.bytes(this.attachment.getContent().length);
		}

		/**
		 * {@inheritDoc}
		 */
		@Override
		public void setLocale(Locale locale) {
			this.locale = locale;
		}

		/**
		 * {@inheritDoc}
		 */
		@Override
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
						ResourceStreamRequestHandler handler = new ResourceStreamRequestHandler(
								resourceStream, attachment.getFilename());
						RequestCycle.get().scheduleRequestHandlerAfterCurrent(
								handler);
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
			protected void onConfigure() {
				super.onConfigure();

				List<Attachment> attachments = getModelObject();
				setVisible(CollectionUtils.isNotEmpty(attachments));
			}
		};
		add(attachments);
	}

	@Override
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new BasicActionButton("folderButtonId",
				"images/folderlist.png", new ResourceModel("label.folder")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Folder folder = getFolderModel().getObject();

				send(getPage(), Broadcast.DEPTH, new FolderSelectedEvent(
						folder, true));
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

		fragment.add(new BasicActionButton("replyButtonId", "images/reply.png",
				new ResourceModel("label.reply")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (replyButtonId)
			}
		});

		fragment.add(new BasicActionButton("forwardButtonId",
				"images/forward.png", new ResourceModel("label.forward")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (forwardButtonId)
			}
		});

		fragment.add(new BasicActionButton("deleteButtonId",
				"images/delete.png", new ResourceModel("label.delete")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Message message = getMessageModel().getObject();

				if (message != null) {
					// Mark message as deleted on the mail provider.
					boolean deleted = getMailService().deleteMessage(message);

					if (deleted) {
						Folder currentFolder = getFolderModel().getObject();
						currentFolder.removeMessage(message);

						send(getPage(), Broadcast.DEPTH,
								new FolderSelectedEvent(currentFolder, true));
					}
				}
			}
		});

		fragment.add(new BasicActionButton("spamButtonId", "images/junk.png",
				new ResourceModel("label.spam")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Message message = getMessageModel().getObject();

				if (message != null) {
					// Mark the message as junk on the mail provider.
					boolean success = getMailService().markMessageJunk(message);

					if (success) {
						Folder currentFolder = getFolderModel().getObject();
						currentFolder.removeMessage(message);

						send(getPage(), Broadcast.DEPTH,
								new FolderSelectedEvent(currentFolder, true));
					}
				}
			}
		});

		fragment.add(new BasicActionButton("previousButtonId",
				"images/previous.png", new ResourceModel("label.previous")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (previousButtonId)
			}
		});

		fragment.add(new BasicActionButton("nextButtonId", "images/next.png",
				new ResourceModel("label.next")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (nextButtonId)
			}
		});

		return fragment;
	}

	/**
	 * @return the mail service.
	 */
	IMailService getMailService() {
		return this.mailService;
	}

	/**
	 * @return the folder model.
	 */
	IModel<Folder> getFolderModel() {
		return this.folderModel;
	}

	/**
	 * @return the message model.
	 */
	IModel<Message> getMessageModel() {
		return this.messageModel;
	}
}
