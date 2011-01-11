package nl.pointless.webmail.web;

import nl.pointless.webmail.web.component.ActionPanel;
import nl.pointless.webmail.web.component.FolderPanel;
import nl.pointless.webmail.web.component.MessageListPanel;
import nl.pointless.webmail.web.component.WebmailPage;

import org.apache.wicket.util.tester.FormTester;
import org.junit.Test;

/**
 * Unit test for {@link WebmailPage}.
 * 
 * @author Peter Postma
 */
public class MainPageTest {

	@Test
	public void renderTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		tester.assertComponent("actionPanelId", ActionPanel.class);
		tester.assertComponent("folderPanelId", FolderPanel.class);
		tester.assertComponent("messageListId", MessageListPanel.class);

		tester.assertVisible("messageListId");
		tester.assertInvisible("messageViewId");
		tester.assertInvisible("messageWriteId");
	}

	@Test
	public void clickFolderTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		tester.clickLink("folderPanelId:foldersId:0:folderLinkId");

		tester.assertVisible("messageListId");
		tester.assertInvisible("messageViewId");
		tester.assertInvisible("messageWriteId");
	}

	@Test
	public void clickMessageTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		tester.clickLink("messageListId:messagesDataTableId:body:rows:1:cells:1:cell:linkId");

		tester.assertInvisible("messageListId");
		tester.assertVisible("messageViewId");
		tester.assertInvisible("messageWriteId");
	}

	@Test
	public void clickWriteTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		FormTester formTester = tester.newFormTester("actionPanelId:formId");
		formTester.submit("fragmentId:writeButtonId:buttonId");

		tester.assertInvisible("messageListId");
		tester.assertInvisible("messageViewId");
		tester.assertVisible("messageWriteId");
	}
}
