package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.PanelSwitcher;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.markup.html.WebPage;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.resources.StyleSheetReference;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.PropertyModel;
import org.apache.wicket.model.ResourceModel;
import org.apache.wicket.model.StringResourceModel;
import org.apache.wicket.spring.injection.annot.SpringBean;

/**
 * The main page for webmail. Shown directly after logging in.<br>
 * Shows the folder listing in the main content screen.
 * 
 * @author Peter Postma
 */
public class WebmailPage extends WebPage {

	@SpringBean
	private IMailService mailService;

	/**
	 * Constructor.
	 */
	public WebmailPage() {
		FolderPanel folderPanel = new FolderPanel("folderPanelId");
		add(folderPanel);

		IModel<Folder> folderModel = new PropertyModel<Folder>(folderPanel,
				"currentFolder");

		MessageListPanel messageListPanel = new MessageListPanel(
				MessageListPanel.PANEL_ID, folderPanel, folderModel);
		add(messageListPanel);

		IModel<Message> messageModel = new PropertyModel<Message>(
				messageListPanel, "selectedMessage");

		MessageViewPanel messageViewPanel = new MessageViewPanel(
				MessageViewPanel.PANEL_ID, folderModel, messageModel);
		add(messageViewPanel);

		MessageWritePanel messageWritePanel = new MessageWritePanel(
				MessageWritePanel.PANEL_ID);
		add(messageWritePanel);

		ActionPanel actionPanel = new ActionPanel("actionPanelId");
		add(actionPanel);

		// Add the message list/view panels to the content panel switcher.
		PanelSwitcher panelSwitcher = WebmailSession.get().getPanelSwitcher();
		panelSwitcher.add(messageListPanel);
		panelSwitcher.add(messageViewPanel);
		panelSwitcher.add(messageWritePanel);
		panelSwitcher.setPanelSwitchListener(actionPanel);
		panelSwitcher.setActivePanel(MessageListPanel.PANEL_ID);

		Label pageTitleLabel = new Label("pageTitleId", new ResourceModel(
				"title.main"));
		add(pageTitleLabel);

		Label webmailLabel = new Label("webmailLabelId", new ResourceModel(
				"title.main"));
		add(webmailLabel);

		Label loggedIn = new Label("loggedInId", new StringResourceModel(
				"label.logged_in", this, null,
				new Object[] { this.mailService.getServiceName() }));
		add(loggedIn);

		Label copyrightLabel = new Label("copyrightLabelId", new ResourceModel(
				"label.copyright"));
		copyrightLabel.setEscapeModelStrings(false);
		add(copyrightLabel);

		add(new StyleSheetReference("cssId", WebmailPage.class,
				"css/webmail.css"));
	}
}
