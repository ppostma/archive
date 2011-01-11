package nl.pointless.webmail.web.component;

import nl.pointless.webmail.service.IAuthenticator;
import nl.pointless.webmail.web.IPanelSwitchListener;
import nl.pointless.webmail.web.ISwitchablePanel;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.Component;
import org.apache.wicket.ResourceReference;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.markup.html.panel.EmptyPanel;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.markup.html.panel.Panel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel containing the user actions, such as write, reply, forward, etc.
 * 
 * @author Peter Postma
 */
public class ActionPanel extends Panel implements IPanelSwitchListener {

	private static final long serialVersionUID = 1L;

	@SpringBean
	private IAuthenticator authenticator;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 */
	public ActionPanel(String id) {
		super(id);

		add(new EmptyPanel("fragmentId"));

		ImageButton logoutButton = new ImageButton("logoutButtonId",
				new ResourceReference(ActionPanel.class, "images/logout.png")) {

			private static final long serialVersionUID = 1L;

			@Override
			public void onSubmit() {
				getAuthenticator().logout();

				WebmailSession.get().invalidateNow();

				setResponsePage(WebmailPage.class);
			}
		};
		add(logoutButton);

		Label logoutLabel = new Label("logoutLabelId", new ResourceModel(
				"label.logout"));
		add(logoutLabel);
	}

	/**
	 * {@inheritDoc}
	 */
	public void onHidePanel(ISwitchablePanel panel) {
		// No implementation
	}

	/**
	 * {@inheritDoc}
	 */
	public void onShowPanel(ISwitchablePanel panel) {
		Fragment fragment = panel.getActionButtons("fragmentId");

		Component component = get("fragmentId");
		component.replaceWith(fragment);
	}

	/**
	 * @return the authenticator
	 */
	IAuthenticator getAuthenticator() {
		return this.authenticator;
	}
}
