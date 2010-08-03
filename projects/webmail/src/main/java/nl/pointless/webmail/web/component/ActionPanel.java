package nl.pointless.webmail.web.component;

import nl.pointless.webmail.service.IAuthenticator;
import nl.pointless.webmail.web.IPanelSwitchListener;
import nl.pointless.webmail.web.ISwitchablePanel;
import nl.pointless.webmail.web.PanelSwitcher;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.ResourceReference;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.ImageButton;
import org.apache.wicket.markup.html.panel.EmptyPanel;
import org.apache.wicket.markup.html.panel.Fragment;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * Panel containing the user actions, such as write, reply, forward, etc.
 * 
 * @author Peter Postma
 */
public class ActionPanel extends AbstractWebmailPanel implements
		IPanelSwitchListener {

	private static final long serialVersionUID = -8970081410920802851L;

	@SpringBean
	private IAuthenticator authenticator;

	private Form<Object> form;

	/**
	 * Constructor.
	 * 
	 * @param id Wicket panel id.
	 * @param panelSwitcher A {@link PanelSwitcher}.
	 */
	public ActionPanel(String id, PanelSwitcher panelSwitcher) {
		super(id, panelSwitcher);

		this.form = new Form<Object>("formId");
		this.form.add(new EmptyPanel("fragmentId"));
		add(this.form);

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
		this.form.add(logoutButton);

		Label logoutLabel = new Label("logoutLabelId", new ResourceModel(
				"label.logout"));
		this.form.add(logoutLabel);
	}

	/**
	 * {@inheritDoc}
	 */
	public void onHidePanel(ISwitchablePanel panel) {
	}

	/**
	 * {@inheritDoc}
	 */
	public void onShowPanel(ISwitchablePanel panel) {
		Fragment fragment = panel.getActionButtons("fragmentId");
		this.form.addOrReplace(fragment);
	}

	/**
	 * @return the authenticator
	 */
	protected IAuthenticator getAuthenticator() {
		return this.authenticator;
	}
}
