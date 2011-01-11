package nl.pointless.webmail.web;

import nl.pointless.webmail.web.component.ActionPanel;
import nl.pointless.webmail.web.component.FolderPanel;
import nl.pointless.webmail.web.component.MessageListPanel;
import nl.pointless.webmail.web.component.WebmailPage;

import org.apache.wicket.util.tester.FormTester;
import org.junit.Before;
import org.junit.Test;

/**
 * Unit test for {@link WebmailPage}.
 * 
 * @author Peter Postma
 */
public class MainPageTest {

	private WebmailTester tester;

	@Before
	public void init() {
		this.tester = new WebmailTester();
	}

	@Test
	public void renderTest() {
		this.tester.startPage(WebmailPage.class);
		this.tester.assertRenderedPage(WebmailPage.class);

		this.tester.assertComponent("mainFormId:actionPanelId",
				ActionPanel.class);
		this.tester.assertComponent("mainFormId:folderPanelId",
				FolderPanel.class);
		this.tester.assertComponent("mainFormId:messageListId",
				MessageListPanel.class);

		this.tester.assertVisible("mainFormId:messageListId");
		this.tester.assertInvisible("mainFormId:messageViewId");
		this.tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickFolderTest() {
		this.tester.startPage(WebmailPage.class);
		this.tester.assertRenderedPage(WebmailPage.class);

		this.tester.clickLink("mainFormId:folderPanelId:foldersId:0:folderLinkId");

		this.tester.assertVisible("mainFormId:messageListId");
		this.tester.assertInvisible("mainFormId:messageViewId");
		this.tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickMessageTest() {
		this.tester.startPage(WebmailPage.class);
		this.tester.assertRenderedPage(WebmailPage.class);

		this.tester.clickLink("mainFormId:messageListId:messagesDataTableId:body:rows:1:cells:1:cell:linkId");

		this.tester.assertInvisible("mainFormId:messageListId");
		this.tester.assertVisible("mainFormId:messageViewId");
		this.tester.assertInvisible("mainFormId:messageWriteId");
	}

	@Test
	public void clickWriteTest() {
		this.tester.startPage(WebmailPage.class);
		this.tester.assertRenderedPage(WebmailPage.class);

		FormTester formTester = this.tester.newFormTester("mainFormId");
		formTester.submit("actionPanelId:fragmentId:writeButtonId:buttonId");

		this.tester.assertInvisible("mainFormId:messageListId");
		this.tester.assertInvisible("mainFormId:messageViewId");
		this.tester.assertVisible("mainFormId:messageWriteId");
	}
}
