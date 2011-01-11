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

		tester.assertComponent("mainFormId:actionPanelId", ActionPanel.class);
		tester.assertComponent("mainFormId:folderPanelId", FolderPanel.class);
		tester.assertComponent("mainFormId:messageListId",
				MessageListPanel.class);

		tester.assertVisible("mainFormId:messageListId");
		tester.assertInvisible("mainFormId:messageViewId");
		tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickFolderTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		tester.clickLink("mainFormId:folderPanelId:foldersId:0:folderLinkId");

		tester.assertVisible("mainFormId:messageListId");
		tester.assertInvisible("mainFormId:messageViewId");
		tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickMessageTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		tester.clickLink("mainFormId:messageListId:messagesDataTableId:body:rows:1:cells:1:cell:linkId");

		tester.assertInvisible("mainFormId:messageListId");
		tester.assertVisible("mainFormId:messageViewId");
		tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickWriteTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);
		tester.assertRenderedPage(WebmailPage.class);

		FormTester formTester = tester.newFormTester("mainFormId");
		formTester.submit("actionPanelId:fragmentId:writeButtonId:buttonId");

		tester.assertInvisible("mainFormId:messageListId");
		tester.assertInvisible("mainFormId:messageViewId");
		tester.assertVisible("mainFormId:messageWriteId");
	}
}
