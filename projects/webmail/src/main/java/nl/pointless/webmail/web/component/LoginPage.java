package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Login;
import nl.pointless.webmail.service.IAuthenticator;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.markup.html.WebPage;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Button;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.PasswordTextField;
import org.apache.wicket.markup.html.form.RequiredTextField;
import org.apache.wicket.markup.html.panel.FeedbackPanel;
import org.apache.wicket.markup.html.resources.StyleSheetReference;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * The webmail login page.
 * 
 * @author Peter Postma
 */
public class LoginPage extends WebPage {

	/**
	 * The form on the login page.
	 * 
	 * @author Peter Postma
	 */
	private class LoginForm extends Form<Login> {

		private static final long serialVersionUID = 1L;

		/**
		 * Constructor.
		 * 
		 * @param id Component id.
		 * @param login Model for login.
		 */
		public LoginForm(String id, IModel<Login> login) {
			super(id);

			removePersistentFormComponentValues(true);

			Label usernameLabel = new Label("usernameLabelId",
					new ResourceModel("label.username"));
			add(usernameLabel);

			RequiredTextField<String> usernameTextField = new RequiredTextField<String>(
					"usernameId", new PropertyModel<String>(login, "username"));
			usernameTextField.setLabel(new ResourceModel("label.username"));
			add(usernameTextField);

			Label passwordLabel = new Label("passwordLabelId",
					new ResourceModel("label.password"));
			add(passwordLabel);

			PasswordTextField passwordTextField = new PasswordTextField(
					"passwordId", new PropertyModel<String>(login, "password"));
			passwordTextField.setLabel(new ResourceModel("label.password"));
			add(passwordTextField);

			Button button = new Button("submitId", new ResourceModel(
					"button.submit"));
			add(button);
		}

		@Override
		protected void onSubmit() {
			Login login = getModelObject();
			boolean authenticated = getAuthenticator().authenticate(login);

			if (authenticated) {
				WebmailSession.get().setUsername(login.getUsername());

				clearInput();

				setResponsePage(WebmailPage.class);
			} else {
				error(getString("error.authenticate"));
			}
		}
	}

	@SpringBean
	private IAuthenticator authenticator;

	/**
	 * Constructor.
	 */
	public LoginPage() {
		Label pageTitleLabel = new Label("pageTitleId", new ResourceModel(
				"title.login"));
		add(pageTitleLabel);

		Label titleLabel = new Label("titleId",
				new ResourceModel("title.login"));
		add(titleLabel);

		Model<Login> loginModel = new Model<Login>(new Login());

		LoginForm form = new LoginForm("formId", loginModel);
		form.setModel(loginModel);
		add(form);

		FeedbackPanel feedbackPanel = new FeedbackPanel("feedbackId");
		add(feedbackPanel);

		add(new StyleSheetReference("cssId", LoginPage.class, "css/login.css"));
	}

	/**
	 * @return the authenticator
	 */
	IAuthenticator getAuthenticator() {
		return this.authenticator;
	}
}
