package nl.pointless.webmail.service.impl;

import javax.mail.AuthenticationFailedException;
import javax.mail.MessagingException;
import javax.mail.Store;

import nl.pointless.webmail.dto.Login;
import nl.pointless.webmail.service.IAuthenticator;

import org.apache.log4j.Logger;

/**
 * Implementation of {@link IAuthenticator}.
 * 
 * @author Peter Postma
 */
public class JavaMailAuthenticator implements IAuthenticator {

	private static final Logger log = Logger
			.getLogger(JavaMailAuthenticator.class);

	private JavaMailSession mailSession;

	/**
	 * {@inheritDoc}
	 */
	public boolean authenticate(Login login) {
		boolean authenticated = false;

		try {
			Store store = this.mailSession.getStore();
			if (store.isConnected()) {
				store.close();
			}

			store.connect(login.getUsername(), login.getPassword());

			authenticated = true;
		} catch (AuthenticationFailedException e) {
			log.info("Authentication failed for '" + login.getUsername() + "'");
		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		}

		return authenticated;
	}

	/**
	 * {@inheritDoc}
	 */
	public void logout() {
		try {
			Store store = this.mailSession.getStore();
			store.close();
		} catch (MessagingException e) {
			log.error(e.getMessage(), e);
		}
	}

	public void setMailSession(JavaMailSession mailSession) {
		this.mailSession = mailSession;
	}
}
