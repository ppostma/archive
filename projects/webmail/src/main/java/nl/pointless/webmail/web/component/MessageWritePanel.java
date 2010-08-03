package nl.pointless.webmail.web.component;

import nl.pointless.webmail.web.PanelSwitcher;

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
public class MessageWritePanel extends AbstractWebmailSwitchablePanel {

	private static final long serialVersionUID = 8205602425658031156L;

	/**
	 * Construct a {@link MessageWritePanel}.
	 * 
	 * @param id Wicket panel id.
	 * @param panelSwitcher a {@link PanelSwitcher}.
	 */
	public MessageWritePanel(String id, PanelSwitcher panelSwitcher) {
		super(id, panelSwitcher);

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
				activatePreviousPanel();
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
