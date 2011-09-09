package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;

import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.TextArea;
import org.apache.wicket.markup.html.form.TextField;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel for writing a message.
 * 
 * @author Peter Postma
 */
public class MessageWritePanel extends AbstractSwitchablePanel {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IMailService mailService;

	private IModel<Message> messageModel;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 */
	public MessageWritePanel(String id) {
		this(id, new Model<Message>(new Message()));
	}

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param messageModel Model with the message.
	 */
	public MessageWritePanel(String id, IModel<Message> messageModel) {
		super(id);
		this.messageModel = messageModel;

		Form<Object> form = new Form<Object>("formId");
		add(form);

		form.add(new TextField<String>("toId", new PropertyModel<String>(
				this.messageModel, "recipient")));
		form.add(new TextField<String>("subjectId", new PropertyModel<String>(
				this.messageModel, "subject")));
		form.add(new TextArea<String>("bodyId", new PropertyModel<String>(
				this.messageModel, "body")));
	}

	@Override
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new BasicActionButton("discardButtonId",
				"images/discard.png", new ResourceModel("label.discard")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO setActivePanelToPreviousPanel (discard)
			}
		});

		fragment.add(new BasicActionButton("sendButtonId", "images/send.png",
				new ResourceModel("label.send")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				Message message = getMessageModel().getObject();
				getMailService().sendMessage(message);
			}
		});

		fragment.add(new BasicActionButton("attachButtonId",
				"images/attach.png", new ResourceModel("label.attach")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (attachButtonId)
			}
		});

		fragment.add(new BasicActionButton("saveButtonId", "images/save.png",
				new ResourceModel("label.save")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me (saveButtonId)
			}
		});

		return fragment;
	}

	/**
	 * @return the mail service
	 */
	IMailService getMailService() {
		return this.mailService;
	}

	/**
	 * @return the message model
	 */
	IModel<Message> getMessageModel() {
		return this.messageModel;
	}
}
