package nl.pointless.commons.web.component;

import java.text.DateFormat;
import java.util.Date;

import org.apache.wicket.extensions.markup.html.repeater.data.table.PropertyColumn;
import org.apache.wicket.model.AbstractReadOnlyModel;
import org.apache.wicket.model.IModel;

/**
 * A {@link PropertyColumn} that outputs a formatted date.
 * 
 * @author Peter Postma
 * @param <T> Type of the property.
 */
public class DatePropertyColumn<T> extends PropertyColumn<T> {

	private static final long serialVersionUID = 1L;

	private final DateFormat formatter;

	/**
	 * Constructor.
	 * 
	 * @param displayModel Display model.
	 * @param propertyExpression Wicket property expression.
	 * @param aFormatter The formatter to be used to convert the date.
	 */
	public DatePropertyColumn(IModel<String> displayModel,
			String propertyExpression, DateFormat aFormatter) {
		super(displayModel, propertyExpression);
		this.formatter = aFormatter;
	}

	@Override
	protected IModel<?> createLabelModel(IModel<T> embeddedModel) {
		final IModel<?> property = super.createLabelModel(embeddedModel);

		return new AbstractReadOnlyModel<String>() {

			private static final long serialVersionUID = 1L;

			@Override
			public String getObject() {
				Date date = (Date) property.getObject();
				return getFormatter().format(date);
			}
		};
	}

	/**
	 * @return the date formatter.
	 */
	final DateFormat getFormatter() {
		return this.formatter;
	}
}
