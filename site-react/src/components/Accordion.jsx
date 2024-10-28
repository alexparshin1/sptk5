import React from "react";
import {Navigate, NavLink} from "react-router-dom";
import "./Accordion.css";

/**
 * @brief Accordion component
 * Props:
 * - menu: list of menu items
 * CSS files:
 * - Accordion.css
 * Callback:
 * - onChange(link) when a menu item is selected
 *
 * The menu is a list of groups. Each group contains a list of items.
 * The first group and item are selected by default.
 *
 * The example of the menu:
 * [
 *  {title: "Group 1", items: [{title: "Home", link: "/home"}, {title: "About", link: "/about"}]},
 *  {title: "Group 2", items: [{title: "Support", link: "/support"}, {title: "Contact", link: "/contact"}]},
 *  {title: "Group 3", items: [{title: "Documentation", link: "/documentation"}, {title: "Reference", link: "/reference"}]}
 * ]
 */
export default class Accordion extends React.Component
{
    state = {
        selectedGroup: "",
        dataVersion: 0
    };


    constructor(props)
    {
        super();
        this.menu = props.menu;
        this.onChange = props.onChange;
        this.state.selectedGroup = this.menu[0].title;

        this.selectedLinks = {};
        for (let group of this.menu) {
            this.selectedLinks[group.title] = group.items[0].link;
        }
    }

    onItemClick(link)
    {
        this.selectedLinks[this.state.selectedGroup] = link;
        this.setState({dataVersion: this.state.dataVersion + 1});
        if (this.onChange) {
            this.onChange(link);
        }
    }

    renderGroup(group, groupIsSelected)
    {
        if (!groupIsSelected) {
            return <div key={"accordion-" + group.title} className="AccordionGroup"
                        onClick={() => this.setState({selectedGroup: group.title})}>{group.title}</div>;
        }

        let items = [];
        for (let item of group.items) {
            let itemIsSelected = item.link === this.selectedLinks[group.title];
            let itemClass = itemIsSelected ? "AccordionItemSelected" : "AccordionItem";
            items.push(
                <div key={item.title + "-item"} className={itemClass}>
                    <NavLink key={item.title + "-navlink"} to={item.link}
                             onClick={() => this.onItemClick(item.link)}>{item.title}</NavLink>
                </div>);
        }
        return <div key={"accordion-group-" + group.title}>
            <div key={"accordion-" + group.title} className="AccordionGroup"
                 onClick={() => this.setState({selectedGroup: group.title})}>{group.title}</div>
            {items}
        </div>;
    }

    render()
    {
        let groups = [];
        for (let group of this.menu) {
            groups.push(this.renderGroup(group, group.title === this.state.selectedGroup));
        }
        return <div key={"accordion"}>
            {groups}
            <Navigate key={"accordion-navigate"} to={this.selectedLinks[this.state.selectedGroup]} replace={true}/>
        </div>;
    }
}
