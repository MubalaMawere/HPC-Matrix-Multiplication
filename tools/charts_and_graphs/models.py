from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from plotly.graph_objects import Figure


@dataclass
class ResultRow:
    index: int
    mode: str
    matrix_size: int
    threads: int
    sample_count: int | None
    sample_times: list[float]
    execution_seconds: float
    speedup: float | None
    efficiency: float | None
    verified: str


@dataclass
class ChartSections:
    sample_chart_html: str
    time_chart_html: str
    scaling_chart_html: str


@dataclass
class ChartFigures:
    sample_figure: Figure
    time_figure: Figure
    scaling_figure: Figure


@dataclass
class BuiltCharts:
    sections: ChartSections
    figures: ChartFigures


@dataclass
class ChartDataFiles:
    summary_csv: Path
    sample_csv: Path


@dataclass
class ChartImageFolders:
    png_dir: Path
    svg_dir: Path
