package nl.pointless.webmail.web.component;

import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.TextArea;
import org.apache.wicket.markup.html.form.TextField;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.model.ResourceModel;

/**
 * Panel for writing a message.
 * 
 * @author Peter Postma
 */
public class MessageWritePanel extends AbstractSwitchablePanel {

	private static final long serialVersionUID = 8205602425658031156L;

	public static final String PANEL_ID = "messageWriteId";

	/**
	 * Construct a {@link MessageWritePanel}.
	 * 
	 * @param id Wicket panel id.
	 */
	public MessageWritePanel(String id) {
		super(id);

		Form<Object> form = new Form<Object>("formId");
		add(form);

		form.add(new TextField<String>("toId"));
		form.add(new TextField<String>("subjectId"));
		form.add(new TextArea<String>("bodyId"));
	}

	/**
	 * {@inheritDoc}
	 */
	protected Fragment createActionButtons(String id) {
		Fragment fragment = new Fragment(id, "actionFragmentId", this);

		fragment.add(new AbstractActionButton("discardButtonId",
				"images/discard.png", new ResourceModel("label.discard")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				WebmailSession.get().getPanelSwitcher()
						.setActivePanelToPreviousPanel();
			}
		});

		fragment.add(new AbstractActionButton("sendButtonId",
				"images/send.png", new ResourceModel("label.send")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		fragment.add(new AbstractActionButton("attachButtonId",
				"images/attach.png", new ResourceModel("label.attach")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		fragment.add(new AbstractActionButton("saveButtonId",
				"images/save.png", new ResourceModel("label.save")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				// TODO Implement me
			}
		});

		return fragment;
	}
}
