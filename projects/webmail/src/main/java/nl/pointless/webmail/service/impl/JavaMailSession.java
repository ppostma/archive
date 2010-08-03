package nl.pointless.webmail.service.impl;

import javax.mail.NoSuchProviderException;
import javax.mail.Session;
import javax.mail.Store;

/**
 * Session object for a JavaMail session.
 * 
 * @author Peter Postma
 */
public class JavaMailSession {

	private Session session;

	private Store store;

	/**
	 * Get a {@link javax.mail.Store} instance.
	 * 
	 * @return Store
	 * @throws NoSuchProviderException
	 */
	public Store getStore() throws NoSuchProviderException {
		if (this.store == null) {
			this.store = this.session.getStore();
		}
		return this.store;
	}

	public void setSession(Session session) {
		this.session = session;
	}
}
