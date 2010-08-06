package nl.pointless.webmail.web;

import nl.pointless.webmail.web.component.ActionPanel;
import nl.pointless.webmail.web.component.FolderPanel;
import nl.pointless.webmail.web.component.MessageListPanel;
import nl.pointless.webmail.web.component.WebmailPage;

import org.junit.Ignore;
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
	@Ignore
	public void clickFolderTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);

		tester.clickLink("folderPanelId:foldersId:0:folderLinkId");

		tester.assertVisible("messageListId");
		tester.assertInvisible("messageViewId");
		tester.assertInvisible("messageWriteId");
	}

	@Test
	@Ignore
	public void clickMessageTest() {
		WebmailTester tester = new WebmailTester();

		tester.startPage(WebmailPage.class);

		tester.clickLink("messageListId:messagesDataTableId:rows:1:cells:1:cell:linkId");

		tester.assertInvisible("messageListId");
		tester.assertVisible("messageViewId");
		tester.assertInvisible("messageWriteId");
	}
}
