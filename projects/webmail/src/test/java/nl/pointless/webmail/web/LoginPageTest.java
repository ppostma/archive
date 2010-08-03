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
import org.junit.Test;

/**
 * Unit test for {@link LoginPage}.
 * 
 * @author Peter Postma
 */
public class LoginPageTest {

	@Test
	public void renderTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(LoginPage.class);
		tester.assertRenderedPage(LoginPage.class);

		tester.assertComponent("formId", Form.class);
		tester.assertComponent("formId:usernameLabelId", Label.class);
		tester.assertComponent("formId:passwordLabelId", Label.class);
		tester.assertComponent("formId:usernameId", RequiredTextField.class);
		tester.assertComponent("formId:passwordId", PasswordTextField.class);
		tester.assertComponent("formId:submitId", Button.class);

		tester.assertComponent("feedbackId", FeedbackPanel.class);
	}

	@Test
	public void loginTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(LoginPage.class);
		tester.assertRenderedPage(LoginPage.class);

		FormTester formTester = tester.newFormTester("formId");
		formTester.setValue("usernameId", "test");
		formTester.setValue("passwordId", "test");
		formTester.submit();

		tester.assertRenderedPage(WebmailPage.class);
	}

	@Test
	public void loginFailTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(LoginPage.class);
		tester.assertRenderedPage(LoginPage.class);

		tester.assertNoErrorMessage();

		FormTester formTester = tester.newFormTester("formId");
		formTester.setValue("usernameId", "test");
		formTester.setValue("passwordId", "invalid");
		formTester.submit();

		tester.assertRenderedPage(LoginPage.class);
		tester
				.assertErrorMessages(new String[] { "Couldn't authenticate, invalid username/password combination." });
	}
}
