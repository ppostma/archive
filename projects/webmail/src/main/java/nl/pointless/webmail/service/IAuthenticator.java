package nl.pointless.webmail.service;

import nl.pointless.webmail.dto.Login;

/**
 * Interface for authenticating an user on the mail server.
 * 
 * @author Peter Postma
 */
public interface IAuthenticator {

	/**
	 * Authenticate the user. Returns <code>true</code> when successfully
	 * authenticated and <code>false</code> otherwise.
	 * 
	 * @param login Login information.
	 * @return <code>true</code> when successfully authenticated.
	 */
	boolean authenticate(Login login);

	/**
	 * Log out the user.
	 */
	void logout();
}
