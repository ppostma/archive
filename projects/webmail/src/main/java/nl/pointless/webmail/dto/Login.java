package nl.pointless.webmail.dto;

import java.io.Serializable;

/**
 * DTO holding login information.
 * 
 * @author Peter Postma
 */
public class Login implements Serializable {

	private static final long serialVersionUID = 4723904996324355863L;

	private String username;
	private String password;

	public String getUsername() {
		return this.username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public String getPassword() {
		return this.password;
	}

	public void setPassword(String password) {
		this.password = password;
	}
}
