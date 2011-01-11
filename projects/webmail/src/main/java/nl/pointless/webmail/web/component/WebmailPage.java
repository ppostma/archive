package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.PanelSwitcher;
import nl.pointless.webmail.web.WebmailSession;

import org.apache.wicket.markup.html.WebPage;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Form;
import org.apache.wicket.markup.html.resources.StyleSheetReference;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
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

	/**
	 * Component id for the message list panel.
	 */
	static final String MESSAGE_LIST_PANEL_ID = "messageListId";

	/**
	 * Component id for the message view panel.
	 */
	static final String MESSAGE_VIEW_PANEL_ID = "messageViewId";

	/**
	 * Component id for the message write panel.
	 */
	static final String MESSAGE_WRITE_PANEL_ID = "messageWriteId";

	@SpringBean
	private IMailService mailService;

	/**
	 * Constructor.
	 */
	public WebmailPage() {
		IModel<Folder> folderModel = new Model<Folder>();
		IModel<Message> messageModel = new Model<Message>();

		Form<Void> mainForm = new Form<Void>("mainFormId");
		add(mainForm);

		FolderPanel folderPanel = new FolderPanel("folderPanelId", folderModel);
		mainForm.add(folderPanel);

		MessageListPanel messageListPanel = new MessageListPanel(
				MESSAGE_LIST_PANEL_ID, folderModel, messageModel);
		messageListPanel.addFolderRefreshActionListener(folderPanel);
		mainForm.add(messageListPanel);

		folderPanel.addFolderSelectListener(messageListPanel);

		MessageViewPanel messageViewPanel = new MessageViewPanel(
				MESSAGE_VIEW_PANEL_ID, folderModel, messageModel);
		mainForm.add(messageViewPanel);

		MessageWritePanel messageWritePanel = new MessageWritePanel(
				MESSAGE_WRITE_PANEL_ID);
		mainForm.add(messageWritePanel);

		ActionPanel actionPanel = new ActionPanel("actionPanelId");
		mainForm.add(actionPanel);

		initializePanelSwitcher(messageListPanel, messageViewPanel,
				messageWritePanel, actionPanel);

		Label webmailLabel = new Label("webmailLabelId", new ResourceModel(
				"title.webmail"));
		mainForm.add(webmailLabel);

		Label loggedIn = new Label("loggedInId", new StringResourceModel(
				"label.logged_in", this, null,
				new Object[] { this.mailService.getServiceName() }));
		mainForm.add(loggedIn);

		Label copyrightLabel = new Label("copyrightLabelId", new ResourceModel(
				"label.copyright"));
		copyrightLabel.setEscapeModelStrings(false);
		mainForm.add(copyrightLabel);

		Label pageTitleLabel = new Label("pageTitleId", new ResourceModel(
				"title.webmail"));
		add(pageTitleLabel);

		add(new StyleSheetReference("cssId", WebmailPage.class,
				"css/webmail.css"));
	}

	/**
	 * Initialize the {@link PanelSwitcher}.
	 * 
	 * @param messageListPanel The {@link MessageListPanel}.
	 * @param messageViewPanel The {@link MessageViewPanel}.
	 * @param messageWritePanel The {@link MessageWritePanel}.
	 * @param actionPanel The {@link ActionPanel}.
	 */
	private void initializePanelSwitcher(MessageListPanel messageListPanel,
			MessageViewPanel messageViewPanel,
			MessageWritePanel messageWritePanel, ActionPanel actionPanel) {
		PanelSwitcher panelSwitcher = new PanelSwitcher();
		panelSwitcher.add(messageListPanel);
		panelSwitcher.add(messageViewPanel);
		panelSwitcher.add(messageWritePanel);
		panelSwitcher.setPanelSwitchListener(actionPanel);
		panelSwitcher.setActivePanel(MESSAGE_LIST_PANEL_ID);

		WebmailSession.get().setPanelSwitcher(panelSwitcher);
	}
}
