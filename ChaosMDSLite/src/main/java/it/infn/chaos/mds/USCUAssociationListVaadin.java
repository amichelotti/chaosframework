package it.infn.chaos.mds;

import com.vaadin.annotations.AutoGenerated;
import com.vaadin.ui.Alignment;
import com.vaadin.ui.Button;
import com.vaadin.ui.CustomComponent;
import com.vaadin.ui.HorizontalLayout;
import com.vaadin.ui.Label;
import com.vaadin.ui.Table;
import com.vaadin.ui.VerticalLayout;

public class USCUAssociationListVaadin extends CustomComponent {

	/*- VaadinEditorProperties={"grid":"RegularGrid,20","showGrid":true,"snapToGrid":true,"snapToObject":true,"movingGuides":false,"snappingDistance":10} */

	@AutoGenerated
	private VerticalLayout		mainLayout;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_3;
	@AutoGenerated
	private VerticalLayout		verticalLayout_2;
	@AutoGenerated
	private HorizontalLayout	horizontalLayout_6;
	@AutoGenerated
	private Button				buttonUpdateList;
	@AutoGenerated
	private Button				buttonRemoveAssociation;
	@AutoGenerated
	private Button				buttonUnloadInstance;
	@AutoGenerated
	private Button				buttonLoadInstance;
	@AutoGenerated
	private Table				tableAssociation;
	@AutoGenerated
	private HorizontalLayout	horizontalLayoutUS;
	@AutoGenerated
	private Label				usSelected;
	@AutoGenerated
	private Label				usLabel;

	/**
	 * The constructor should first build the main layout, set the composition root and then do any custom initialization.
	 * 
	 * The constructor will not be automatically regenerated by the visual editor.
	 */
	public USCUAssociationListVaadin() {
		buildMainLayout();
		setCompositionRoot(mainLayout);

		// TODO add user code here
	}

	@AutoGenerated
	private VerticalLayout buildMainLayout() {
		// common part: create layout
		mainLayout = new VerticalLayout();
		mainLayout.setImmediate(false);
		mainLayout.setWidth("100%");
		mainLayout.setHeight("100%");
		mainLayout.setMargin(false);

		// top-level component properties
		setWidth("100.0%");
		setHeight("100.0%");

		// horizontalLayoutUS
		horizontalLayoutUS = buildHorizontalLayoutUS();
		mainLayout.addComponent(horizontalLayoutUS);

		// horizontalLayout_3
		horizontalLayout_3 = buildHorizontalLayout_3();
		mainLayout.addComponent(horizontalLayout_3);

		return mainLayout;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayoutUS() {
		// common part: create layout
		horizontalLayoutUS = new HorizontalLayout();
		horizontalLayoutUS.setImmediate(false);
		horizontalLayoutUS.setWidth("-1px");
		horizontalLayoutUS.setHeight("-1px");
		horizontalLayoutUS.setMargin(false);
		horizontalLayoutUS.setSpacing(true);

		// usLabel
		usLabel = new Label();
		usLabel.setImmediate(false);
		usLabel.setWidth("-1px");
		usLabel.setHeight("-1px");
		usLabel.setValue("Unit Server:");
		horizontalLayoutUS.addComponent(usLabel);

		// usSelected
		usSelected = new Label();
		usSelected.setImmediate(false);
		usSelected.setWidth("100.0%");
		usSelected.setHeight("-1px");
		usSelected.setValue("Unit Server Selected");
		horizontalLayoutUS.addComponent(usSelected);
		horizontalLayoutUS.setExpandRatio(usSelected, 1.0f);

		return horizontalLayoutUS;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_3() {
		// common part: create layout
		horizontalLayout_3 = new HorizontalLayout();
		horizontalLayout_3.setImmediate(false);
		horizontalLayout_3.setWidth("100.0%");
		horizontalLayout_3.setHeight("-1px");
		horizontalLayout_3.setMargin(false);

		// verticalLayout_2
		verticalLayout_2 = buildVerticalLayout_2();
		horizontalLayout_3.addComponent(verticalLayout_2);

		return horizontalLayout_3;
	}

	@AutoGenerated
	private VerticalLayout buildVerticalLayout_2() {
		// common part: create layout
		verticalLayout_2 = new VerticalLayout();
		verticalLayout_2.setImmediate(false);
		verticalLayout_2.setWidth("100.0%");
		verticalLayout_2.setHeight("-1px");
		verticalLayout_2.setMargin(false);

		// tableAssociation
		tableAssociation = new Table();
		tableAssociation.setCaption("Work Unit Associations");
		tableAssociation.setImmediate(false);
		tableAssociation.setWidth("100.0%");
		tableAssociation.setHeight("-1px");
		verticalLayout_2.addComponent(tableAssociation);

		// horizontalLayout_6
		horizontalLayout_6 = buildHorizontalLayout_6();
		verticalLayout_2.addComponent(horizontalLayout_6);

		return verticalLayout_2;
	}

	@AutoGenerated
	private HorizontalLayout buildHorizontalLayout_6() {
		// common part: create layout
		horizontalLayout_6 = new HorizontalLayout();
		horizontalLayout_6.setImmediate(false);
		horizontalLayout_6.setWidth("-1px");
		horizontalLayout_6.setHeight("-1px");
		horizontalLayout_6.setMargin(true);
		horizontalLayout_6.setSpacing(true);

		// buttonLoadInstance
		buttonLoadInstance = new Button();
		buttonLoadInstance.setCaption("Load Instance");
		buttonLoadInstance.setImmediate(true);
		buttonLoadInstance.setWidth("-1px");
		buttonLoadInstance.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonLoadInstance);
		horizontalLayout_6.setComponentAlignment(buttonLoadInstance, new Alignment(33));

		// buttonUnloadInstance
		buttonUnloadInstance = new Button();
		buttonUnloadInstance.setCaption("Unload Instance");
		buttonUnloadInstance.setImmediate(true);
		buttonUnloadInstance.setWidth("-1px");
		buttonUnloadInstance.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonUnloadInstance);

		// buttonRemoveAssociation
		buttonRemoveAssociation = new Button();
		buttonRemoveAssociation.setCaption("Remove Association");
		buttonRemoveAssociation.setImmediate(true);
		buttonRemoveAssociation.setWidth("-1px");
		buttonRemoveAssociation.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonRemoveAssociation);
		horizontalLayout_6.setComponentAlignment(buttonRemoveAssociation, new Alignment(33));

		// buttonUpdateList
		buttonUpdateList = new Button();
		buttonUpdateList.setCaption("Update List");
		buttonUpdateList.setImmediate(false);
		buttonUpdateList.setWidth("-1px");
		buttonUpdateList.setHeight("-1px");
		horizontalLayout_6.addComponent(buttonUpdateList);

		return horizontalLayout_6;
	}

	public Button getButtonRemoveAssociation() {
		return buttonRemoveAssociation;
	}

	public Button getButtonUnloadInstance() {
		return buttonUnloadInstance;
	}

	public Button getButtonLoadInstance() {
		return buttonLoadInstance;
	}

	public Table getTableAssociation() {
		return tableAssociation;
	}

	public Label getUsSelected() {
		return usSelected;
	}

	public Button getButtonUpdateList() {
		return buttonUpdateList;
	}
}
