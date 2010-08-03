package nl.pointless.webmail.web.component;

import java.util.List;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;

import org.apache.wicket.extensions.markup.html.repeater.data.table.DataTable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.HeadersToolbar;
import org.apache.wicket.extensions.markup.html.repeater.data.table.IColumn;
import org.apache.wicket.extensions.markup.html.repeater.data.table.ISortableDataProvider;
import org.apache.wicket.extensions.markup.html.repeater.data.table.NoRecordsToolbar;
import org.apache.wicket.markup.ComponentTag;
import org.apache.wicket.markup.repeater.Item;
import org.apache.wicket.markup.repeater.OddEvenItem;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.StringResourceModel;

/**
 * DataTable for {@link Message} objects.
 * 
 * @author Peter Postma
 */
public class MessageListDataTable extends DataTable<Message> {

	private static final long serialVersionUID = 6482899046896238350L;

	private IModel<Folder> folderModel;

	/**
	 * Constructor.
	 * 
	 * @param id Component id.
	 * @param folderModel The folder model of the folder being listed.
	 * @param columns List of columns.
	 * @param dataProvider The data provider.
	 * @param rowsPerPage Number of rows per page.
	 */
	public MessageListDataTable(String id, IModel<Folder> folderModel,
			List<IColumn<Message>> columns,
			ISortableDataProvider<Message> dataProvider, int rowsPerPage) {
		super(id, columns.toArray(new IColumn[columns.size()]), dataProvider,
				rowsPerPage);
		this.folderModel = folderModel;

		addTopToolbar(new MessageListNavigationToolbar(this));
		addTopToolbar(new HeadersToolbar(this, dataProvider));
		addBottomToolbar(new NoRecordsToolbar(this, new StringResourceModel(
				"label.no_messages", this, this.folderModel)));
	}

	@Override
	protected Item<Message> newRowItem(String id, int index,
			final IModel<Message> model) {
		Item<Message> item = new OddEvenItem<Message>(id, index, model) {

			private static final long serialVersionUID = 1L;

			@Override
			protected void onComponentTag(ComponentTag tag) {
				super.onComponentTag(tag);

				// Add unread CSS class if the message is not read.
				if (!getModelObject().isRead()) {
					String classValue = (String) tag.getString("class");
					tag.put("class", classValue + " unread");
				}
			}
		};

		return item;
	}

	/**
	 * @return folder model
	 */
	public IModel<Folder> getFolderModel() {
		return this.folderModel;
	}
}