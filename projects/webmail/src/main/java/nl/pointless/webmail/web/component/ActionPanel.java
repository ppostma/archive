package nl.pointless.webmail.web.component;

import nl.pointless.webmail.service.IAuthenticator;
import nl.pointless.webmail.web.ISwitchablePanel;
import nl.pointless.webmail.web.WebmailSession;
import nl.pointless.webmail.web.event.PanelShowEvent;

import org.apache.wicket.Component;
import org.apache.wicket.event.IEvent;
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
public class ActionPanel extends Panel {

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

		add(new BasicActionButton("logoutButtonId", "images/logout.png",
				new ResourceModel("label.logout")) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onClick() {
				getAuthenticator().logout();

				WebmailSession.get().invalidateNow();

				setResponsePage(LoginPage.class);
			}
		});
	}

	@Override
	public void onEvent(IEvent<?> event) {
		if (event.getPayload() instanceof PanelShowEvent) {
			ISwitchablePanel panel = ((PanelShowEvent) event.getPayload())
					.getPanel();

			Fragment fragment = panel.getActionButtons("fragmentId");

			Component component = get("fragmentId");
			component.replaceWith(fragment);
		}
	}

	/**
	 * @return the authenticator
	 */
	IAuthenticator getAuthenticator() {
		return this.authenticator;
	}
}
