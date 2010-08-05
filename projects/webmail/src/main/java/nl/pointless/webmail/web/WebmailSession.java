package nl.pointless.webmail.web;

import org.apache.wicket.Request;
import org.apache.wicket.Session;
import org.apache.wicket.protocol.http.WebSession;

/**
 * Session for the webmail application.
 * 
 * @author Peter Postma
 */
public class WebmailSession extends WebSession {

	private static final long serialVersionUID = 4116300316941237042L;

	private String username;

	private PanelSwitcher panelSwitcher;

	/**
	 * Constructor.
	 * 
	 * @param request The current request.
	 */
	public WebmailSession(Request request) {
		super(request);
		this.panelSwitcher = new PanelSwitcher();
	}

	/**
	 * Get the session for the calling thread.
	 * 
	 * @return Session for calling thread
	 */
	public static WebmailSession get() {
		return (WebmailSession) Session.get();
	}

	/**
	 * @return <code>true</code> when authenticated
	 */
	public boolean isAuthenticated() {
		return this.username != null;
	}

	public String getUsername() {
		return this.username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public PanelSwitcher getPanelSwitcher() {
		return panelSwitcher;
	}
}
