package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;
import nl.pointless.webmail.service.IMailService;
import nl.pointless.webmail.web.PanelSwitcher;
import nl.pointless.webmail.web.event.FolderSelectedEvent;
import nl.pointless.webmail.web.event.MessageComposeEvent;
import nl.pointless.webmail.web.event.MessageSelectedEvent;

import org.apache.wicket.event.IEvent;
import org.apache.wicket.markup.html.IHeaderResponse;
import org.apache.wicket.markup.html.WebPage;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.form.Form;
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

	private static final long serialVersionUID = 1L;

	/**
	 * Component id for the message list panel.
	 */
	private static final String MESSAGE_LIST_PANEL_ID = "messageListId";

	/**
	 * Component id for the message view panel.
	 */
	private static final String MESSAGE_VIEW_PANEL_ID = "messageViewId";

	/**
	 * Component id for the message write panel.
	 */
	private static final String MESSAGE_WRITE_PANEL_ID = "messageWriteId";

	private PanelSwitcher panelSwitcher;

	@SpringBean
	private IMailService mailService;

	private IModel<Folder> folderModel;
	private IModel<Message> messageModel;

	/**
	 * Constructor.
	 */
	public WebmailPage() {
		this.folderModel = new Model<Folder>();
		this.messageModel = new Model<Message>();

		Form<Object> mainForm = new Form<Object>("mainFormId");
		add(mainForm);

		FolderPanel folderPanel = new FolderPanel("folderPanelId",
				this.folderModel);
		mainForm.add(folderPanel);

		MessageListPanel messageListPanel = new MessageListPanel(
				MESSAGE_LIST_PANEL_ID, this.folderModel, this.messageModel);
		mainForm.add(messageListPanel);

		MessageViewPanel messageViewPanel = new MessageViewPanel(
				MESSAGE_VIEW_PANEL_ID, this.folderModel, this.messageModel);
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
	}

	@Override
	public void renderHead(IHeaderResponse response) {
		response.renderCSSReference("css/webmail.css");
		response.renderCSSReference("css/jquery-ui-1.8.16.custom.css");

		response.renderJavaScriptReference("js/jquery-1.6.2.min.js");
		response.renderJavaScriptReference("js/jquery-ui-1.8.16.custom.min.js");
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
		this.panelSwitcher = new PanelSwitcher(this);
		this.panelSwitcher.add(messageListPanel);
		this.panelSwitcher.add(messageViewPanel);
		this.panelSwitcher.add(messageWritePanel);
		this.panelSwitcher.setActivePanel(MESSAGE_LIST_PANEL_ID);
	}

	@Override
	public void onEvent(IEvent<?> event) {
		Object payload = event.getPayload();

		if (payload instanceof FolderSelectedEvent) {
			this.panelSwitcher.setActivePanel(MESSAGE_LIST_PANEL_ID);

		} else if (payload instanceof MessageSelectedEvent) {
			this.panelSwitcher.setActivePanel(MESSAGE_VIEW_PANEL_ID);

		} else if (payload instanceof MessageComposeEvent) {
			this.panelSwitcher.setActivePanel(MESSAGE_WRITE_PANEL_ID);
		}
	}
}
