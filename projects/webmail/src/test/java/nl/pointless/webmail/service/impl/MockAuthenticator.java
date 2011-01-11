package nl.pointless.webmail.service.impl;

import nl.pointless.webmail.dto.Login;
import nl.pointless.webmail.service.IAuthenticator;

/**
 * Test implementation for {@link IAuthenticator}.
 * 
 * @author Peter Postma
 */
public class MockAuthenticator implements IAuthenticator {

	/**
	 * {@inheritDoc}
	 */
	public boolean authenticate(Login login) {
		if (login.getUsername().equals("test")
				&& login.getPassword().equals("test")) {
			return true;
		}
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public void logout() {
		// No implementation
	}
}
