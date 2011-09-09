package nl.pointless.webmail.web.component;

import nl.pointless.webmail.dto.Folder;
import nl.pointless.webmail.dto.Message;

import org.apache.commons.lang.StringUtils;
import org.apache.wicket.IClusterable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.DataTable;
import org.apache.wicket.extensions.markup.html.repeater.data.table.NavigationToolbar;
import org.apache.wicket.markup.html.WebComponent;
import org.apache.wicket.markup.html.basic.Label;
import org.apache.wicket.markup.html.navigation.paging.PagingNavigator;
import org.apache.wicket.model.AbstractReadOnlyModel;
import org.apache.wicket.model.IModel;
import org.apache.wicket.model.Model;
import org.apache.wicket.model.StringResourceModel;

/**
 * NavigationToolbar used by {@link MessageListDataTable}.
 * 
 * @author Peter Postma
 */
public class MessageListNavigationToolbar extends NavigationToolbar {

	private static final long serialVersionUID = 1L;

	private IModel<Folder> folderModel;

	/**
	 * Constructor.
	 * 
	 * @param dataTable The message list datatable.
	 * @param folderModel Folder model.
	 */
	public MessageListNavigationToolbar(DataTable<Message> dataTable,
			IModel<Folder> folderModel) {
		super(dataTable);
		this.folderModel = folderModel;
	}

	@Override
	protected PagingNavigator newPagingNavigator(String navigatorId,
			DataTable<?> table) {
		return new MessageListPagingNavigator(navigatorId, table);
	}

	@Override
	protected WebComponent newNavigatorLabel(String navigatorId,
			final DataTable<?> table) {
		return new Label(navigatorId, new AbstractReadOnlyModel<String>() {

			private static final long serialVersionUID = 1L;

			@Override
			public String getObject() {
				MessageListDataProvider dataProvider = (MessageListDataProvider) table
						.getDataProvider();
				String filter = dataProvider.getFilter();

				IModel<LabelModelObject> model = new Model<LabelModelObject>(
						new LabelModelObject(table));
				StringBuilder sb = new StringBuilder();

				if (dataProvider.size() > 0) {
					IModel<String> navigatorModel = new StringResourceModel(
							"label.navigator",
							MessageListNavigationToolbar.this, model);
					sb.append(navigatorModel.getObject());
				}

				sb.append(" ");

				if (StringUtils.isNotEmpty(filter)) {
					IModel<String> filterModel = new StringResourceModel(
							"label.filter", MessageListNavigationToolbar.this,
							model);
					sb.append(filterModel.getObject());
				}

				return sb.toString();
			}
		});
	}

	@Override
	protected void onConfigure() {
		super.onConfigure();

		setVisible(true);
	}

	/**
	 * @return the folder model
	 */
	IModel<Folder> getFolderModel() {
		return this.folderModel;
	}

	/**
	 * Object for to be used for the label model.
	 * 
	 * @author Peter Postma
	 */
	class LabelModelObject implements IClusterable {

		private static final long serialVersionUID = 1L;

		private final DataTable<?> dataTable;

		/**
		 * Construct.
		 * 
		 * @param dataTable The data table.
		 */
		public LabelModelObject(DataTable<?> dataTable) {
			this.dataTable = dataTable;
		}

		/**
		 * @return "z" in "Showing x to y of z"
		 */
		public int getOf() {
			return this.dataTable.getItemCount();
		}

		/**
		 * @return "x" in "Showing x to y of z"
		 */
		public int getFrom() {
			if (getOf() == 0) {
				return 0;
			}
			return this.dataTable.getCurrentPage()
					* this.dataTable.getItemsPerPage() + 1;
		}

		/**
		 * @return "y" in "Showing x to y of z"
		 */
		public int getTo() {
			if (getOf() == 0) {
				return 0;
			}
			return Math.min(getOf(),
					getFrom() + this.dataTable.getItemsPerPage() - 1);
		}

		/**
		 * @return the name of the open folder
		 */
		public String getFolderName() {
			return getFolderModel().getObject().getName();
		}

		/**
		 * @return the search filter
		 */
		public String getSearchFilter() {
			MessageListDataProvider dataProvider = (MessageListDataProvider) this.dataTable
					.getDataProvider();
			return dataProvider.getFilter();
		}
	}
}