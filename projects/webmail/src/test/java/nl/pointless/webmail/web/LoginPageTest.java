package nl.pointless.webmail.web;

import nl.pointless.webmail.web.component.LoginPage;
import nl.pointless.webmail.web.component.WebmailPage;

import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Button;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.form.PasswordTextField;
import org.apache.wicket.markup.html.form.RequiredTextField;
import org.apache.wicket.markup.html.panel.FeedbackPanel;
import org.apache.wicket.util.tester.FormTester;
import org.junit.Before;
import org.junit.Test;

/**
 * Unit test for {@link LoginPage}.
 * 
 * @author Peter Postma
 */
public class LoginPageTest {

	private WebmailTester tester;

	@Before
	public void init() {
		this.tester = new WebmailTester();
	}

	@Test
	public void renderTest() {
		this.tester.startPage(LoginPage.class);
		this.tester.assertRenderedPage(LoginPage.class);

		this.tester.assertComponent("formId", Form.class);
		this.tester.assertComponent("formId:usernameLabelId", Label.class);
		this.tester.assertComponent("formId:passwordLabelId", Label.class);
		this.tester.assertComponent("formId:usernameId", RequiredTextField.class);
		this.tester.assertComponent("formId:passwordId", PasswordTextField.class);
		this.tester.assertComponent("formId:submitId", Button.class);

		this.tester.assertComponent("feedbackId", FeedbackPanel.class);
	}

	@Test
	public void loginTest() {
		this.tester.startPage(LoginPage.class);
		this.tester.assertRenderedPage(LoginPage.class);

		FormTester formTester = this.tester.newFormTester("formId");
		formTester.setValue("usernameId", "test");
		formTester.setValue("passwordId", "test");
		formTester.submit();

		this.tester.assertRenderedPage(WebmailPage.class);
	}

	@Test
	public void loginFailTest() {
		this.tester.startPage(LoginPage.class);
		this.tester.assertRenderedPage(LoginPage.class);

		this.tester.assertNoErrorMessage();

		FormTester formTester = this.tester.newFormTester("formId");
		formTester.setValue("usernameId", "test");
		formTester.setValue("passwordId", "invalid");
		formTester.submit();

		this.tester.assertRenderedPage(LoginPage.class);
		this.tester.assertErrorMessages(new String[] { "Couldn't authenticate, invalid username/password combination." });
	}
}
