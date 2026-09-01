use eframe::{
    egui::{Align, Checkbox, Id, Layout, Ui, UiBuilder, collapsing_header::CollapsingState},
    epaint::Hsva,
};
use egui_plot::PlotMemory;
use fixedbitset::FixedBitSet;

use crate::gui::plot::PlotInfo;

pub(super) fn render(
    ui: &mut Ui,
    plot_info: PlotInfo,
    expanded_legend_group: &mut Option<Id>,
    hidden_entries: &mut FixedBitSet,
) {
    let Some(mut mem) = PlotMemory::load(ui, plot_info.id) else {
        return;
    };

    let cached_width = ui
        .data(|data| data.get_temp(Id::new("legend_width")))
        .unwrap_or_else(|| {
            let mut ui = Ui::new(ui.clone(), Id::new("measure"), UiBuilder::new());
            render_body(
                &mut ui,
                &mut mem,
                0,
                plot_info.legend_entries[0],
                hidden_entries,
            );
            let legend_width = ui.min_rect().width();
            ui.data_mut(|data| data.insert_temp(Id::new("legend_width"), legend_width));
            legend_width
        });
    ui.with_layout(Layout::top_down(Align::RIGHT), |ui| {
        ui.set_min_width(cached_width);
        ui.heading(plot_info.title);

        mem.hovered_legend_item = None;
        let mut offset = 0;
        plot_info
            .legend_entries
            .iter()
            .enumerate()
            .for_each(|(daisy, group)| {
                render_group(
                    ui,
                    &mut mem,
                    plot_info.id,
                    daisy,
                    group,
                    &mut offset,
                    expanded_legend_group,
                    hidden_entries,
                )
            });
    });
    mem.store(ui, plot_info.id);
}

fn render_group(
    ui: &mut Ui,
    mem: &mut PlotMemory,
    plot_id: Id,
    daisy: usize,
    group: &[(&str, Hsva)],
    offset: &mut usize,
    expanded_legend_group: &mut Option<Id>,
    hidden_entries: &mut FixedBitSet,
) {
    let collapse_id = Id::new((plot_id, daisy));
    let mut state = CollapsingState::load_with_default_open(ui, collapse_id, false);
    state.set_open(*expanded_legend_group == Some(collapse_id));
    let state = state.show_header(ui, |ui| {
        let all_hidden = hidden_entries.contains_all_in_range(*offset..*offset + group.len());
        let mut none_hidden = !hidden_entries.contains_any_in_range(*offset..*offset + group.len());
        let indeterminate = !(all_hidden | none_hidden);
        if ui
            .add(
                Checkbox::new(&mut none_hidden, format!("Daisy #{}", daisy + 1))
                    .indeterminate(indeterminate),
            )
            .changed()
        {
            if none_hidden {
                group.iter().for_each(|entry| {
                    mem.hidden_items.remove(&Id::new(entry.0));
                });
                hidden_entries.remove_range(*offset..*offset + group.len());
            } else {
                group.iter().for_each(|entry| {
                    mem.hidden_items.insert(Id::new(entry.0));
                });
                hidden_entries.insert_range(*offset..*offset + group.len());
            }
        }
    });
    if state.is_open() {
        *expanded_legend_group = Some(collapse_id);
    } else if *expanded_legend_group == Some(collapse_id) {
        *expanded_legend_group = None
    }
    state.body_unindented(|ui| render_body(ui, mem, *offset, group, hidden_entries));
    *offset += group.len();
}

fn render_body(
    ui: &mut Ui,
    mem: &mut PlotMemory,
    offset: usize,
    group: &[(&str, Hsva)],
    hidden_entries: &mut FixedBitSet,
) {
    const COLUMNS: usize = 2;
    group.chunks(COLUMNS).enumerate().for_each(|(row, group)| {
        ui.horizontal(|ui| {
            group.iter().enumerate().rev().for_each(|(idx, entry)| {
                let idx = offset + row * COLUMNS + idx;
                let mut checked = !hidden_entries[idx];
                let response = ui.checkbox(&mut checked, &entry.0["Daisy #0 ".len()..]);
                if response.changed() {
                    let id = Id::new(entry.0);
                    if checked {
                        mem.hidden_items.remove(&id);
                        hidden_entries.remove(idx);
                    } else {
                        mem.hidden_items.insert(id);
                        hidden_entries.insert(idx);
                    }
                } else if response.hovered() {
                    let id = Id::new(entry.0);
                    mem.hovered_legend_item = Some(id);
                }
            })
        });
    });
}
