import React from "react";
import "../css/Documentation.css";

export default class ComboBox extends React.Component
{
    render()
    {
        let items = [];
        for (let item of this.props.items)
        {
            items.push(<option key={this.props.name + "-" + item.value} value={item.value}>{item.text}</option>);
        }
        // The list of options changes with the other combo box, so the selection is kept in
        // the caller's state rather than left to the browser
        const selection = this.props.value === undefined ? {} : {value: this.props.value};
        return <select style={{padding: 4, borderRadius: 4}} {...selection}
                       onChange={(e) => this.props.onChange(e.target.value)}>
            {items}
        </select>;
    }
}