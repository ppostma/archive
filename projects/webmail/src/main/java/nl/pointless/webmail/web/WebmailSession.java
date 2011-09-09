package nl.pointless.webmail.web;

import org.apache.wicket.Session;
import org.apache.wicket.protocol.http.WebSession;
import org.apache.wicket.request.Request;

/**
 * Session for the webmail application.
 * 
 * @author Peter Postma
 */
public class WebmailSession extends WebSession {

	private static final long serialVersionUID = 1L;

	private String username;

	/**
	 * Constructor.
	 * 
	 * @param request The current request.
	 */
	public WebmailSession(Request request) {
		super(request);
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
}
