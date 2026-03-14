from __future__ import annotations

import html
from pathlib import Path

from plotly.offline import get_plotlyjs

from charts_and_graphs.models import ChartDataFiles, ChartSections, ResultRow


class ReportBuilder:
    def __init__(self, output_dir: Path) -> None:
        self.output_dir = output_dir
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def write_report(
        self,
        csv_path: Path,
        grouped_rows: dict[int, list[ResultRow]],
        chart_sections: dict[int, ChartSections],
        chart_data_files: ChartDataFiles,
    ) -> Path:
        overview_path = self.output_dir / "performance_report.html"
        sample_page_path = self.output_dir / "sample_times.html"
        average_page_path = self.output_dir / "average_time_comparison.html"
        scaling_page_path = self.output_dir / "why_speedup_is_not_linear.html"

        overview_path.write_text(
            self._build_overview_page(csv_path, grouped_rows, chart_data_files),
            encoding="utf-8",
        )
        sample_page_path.write_text(
            self._build_chart_page(
                title="Sample Times",
                subtitle="Each page section shows only the sample chart for one matrix size.",
                active_page="sample",
                grouped_rows=grouped_rows,
                chart_sections=chart_sections,
                aim_text="Aim: show every recorded sample for each run type in its own panel, with the average time marked by the dashed line.",
                chart_selector=lambda sections: sections.sample_chart_html,
            ),
            encoding="utf-8",
        )
        average_page_path.write_text(
            self._build_chart_page(
                title="Average Time Comparison",
                subtitle="This page focuses only on average time, so the comparison is easier to read.",
                active_page="average",
                grouped_rows=grouped_rows,
                chart_sections=chart_sections,
                aim_text="Aim: compare the average execution time of serial and parallel runs. Lower time is better.",
                chart_selector=lambda sections: sections.time_chart_html,
            ),
            encoding="utf-8",
        )
        scaling_page_path.write_text(
            self._build_chart_page(
                title="Why Speedup Is Not Linear",
                subtitle="This page focuses only on measured speedup versus the ideal straight line.",
                active_page="scaling",
                grouped_rows=grouped_rows,
                chart_sections=chart_sections,
                aim_text="Aim: compare measured speedup with the ideal straight line. The gap comes from thread overhead, memory access cost, and coordination cost.",
                chart_selector=lambda sections: sections.scaling_chart_html,
            ),
            encoding="utf-8",
        )

        return overview_path

    def _build_overview_page(
        self,
        csv_path: Path,
        grouped_rows: dict[int, list[ResultRow]],
        chart_data_files: ChartDataFiles,
    ) -> str:
        sections = [self._build_table_section(matrix_size, rows) for matrix_size, rows in grouped_rows.items()]

        body = f"""
<section class="hero-card">
  <p class="eyebrow">CS421 Parallel Programming</p>
  <h1>Matrix Performance Report</h1>
  <p class="page-subtitle">Use the navigation buttons to open each chart on its own page. This overview page keeps the measured tables and formulas together.</p>
  <div class="meta-row">
    <span class="meta-pill">Raw results: {html.escape(csv_path.name)}</span>
    <span class="meta-pill">Summary data: {html.escape(chart_data_files.summary_csv.name)}</span>
    <span class="meta-pill">Sample data: {html.escape(chart_data_files.sample_csv.name)}</span>
  </div>
</section>

<section class="info-card">
  <h2>How To Read This Report</h2>
  <ul>
    <li><strong>Serial</strong> means one-thread execution.</li>
    <li><strong>Parallel</strong> means OpenMP execution with the shown thread count.</li>
    <li><strong>Runs Averaged</strong> shows how many sample runs were used to get the average time.</li>
    <li><strong>Verified</strong> means the parallel result matched the serial result.</li>
  </ul>
  <p><strong>Formulas</strong></p>
  <div class="formula">average time = (sample1 + sample2 + ... + sampleN) / N</div><br>
  <div class="formula">speedup = serial average time / parallel average time</div><br>
  <div class="formula">efficiency = speedup / number of threads</div>
</section>

<section>
  <h2>Chart Pages</h2>
  <div class="link-grid">
    <a class="link-card" href="sample_times.html">
      <strong>Sample Times</strong>
      <span>See each sample run on its own chart page.</span>
    </a>
    <a class="link-card" href="average_time_comparison.html">
      <strong>Average Time Comparison</strong>
      <span>See only the average-time comparison chart.</span>
    </a>
    <a class="link-card" href="why_speedup_is_not_linear.html">
      <strong>Why Speedup Is Not Linear</strong>
      <span>See only the scaling chart and the gap from ideal speedup.</span>
    </a>
  </div>
</section>

{"".join(sections)}

<p class="data-note">The report uses the latest comparable rows for each matrix size, so serial and parallel values come from the same measured run set.</p>
"""
        return self._build_page(
            title="Matrix Performance Report",
            subtitle="Overview",
            active_page="overview",
            body=body,
        )

    def _build_chart_page(
        self,
        title: str,
        subtitle: str,
        active_page: str,
        grouped_rows: dict[int, list[ResultRow]],
        chart_sections: dict[int, ChartSections],
        aim_text: str,
        chart_selector,
    ) -> str:
        sections = [
            self._build_single_chart_section(
                matrix_size=matrix_size,
                rows=rows,
                chart_title=title,
                aim_text=aim_text,
                chart_html=chart_selector(chart_sections[matrix_size]),
            )
            for matrix_size, rows in grouped_rows.items()
        ]

        body = f"""
<section class="hero-card">
  <p class="eyebrow">CS421 Parallel Programming</p>
  <h1>{html.escape(title)}</h1>
  <p class="page-subtitle">{html.escape(subtitle)}</p>
</section>

{"".join(sections)}
"""
        return self._build_page(
            title=title,
            subtitle=subtitle,
            active_page=active_page,
            body=body,
        )

    def _build_page(self, title: str, subtitle: str, active_page: str, body: str) -> str:
        return f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>{html.escape(title)}</title>
  <script>{get_plotlyjs()}</script>
  <style>
    :root {{
      --bg: #f8fafc;
      --surface: #ffffff;
      --surface-soft: #eff6ff;
      --border: #dbeafe;
      --border-strong: #cbd5e1;
      --text: #0f172a;
      --muted: #475569;
      --accent: #2563eb;
      --accent-soft: #dbeafe;
      --shadow: 0 10px 28px rgba(15, 23, 42, 0.08);
    }}
    * {{
      box-sizing: border-box;
    }}
    body {{
      margin: 0;
      font-family: Arial, sans-serif;
      color: var(--text);
      background: linear-gradient(180deg, #eef6ff 0%, var(--bg) 180px);
    }}
    .page {{
      width: min(1200px, calc(100% - 32px));
      margin: 0 auto;
      padding: 24px 0 40px;
    }}
    .topbar {{
      position: sticky;
      top: 0;
      z-index: 10;
      backdrop-filter: blur(10px);
      background: rgba(248, 250, 252, 0.86);
      border-bottom: 1px solid rgba(203, 213, 225, 0.9);
    }}
    .topbar-inner {{
      width: min(1200px, calc(100% - 32px));
      margin: 0 auto;
      padding: 14px 0;
      display: flex;
      flex-wrap: wrap;
      align-items: center;
      justify-content: space-between;
      gap: 14px;
    }}
    .brand {{
      font-weight: 700;
      letter-spacing: 0.01em;
    }}
    .nav-buttons {{
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
    }}
    .nav-button {{
      text-decoration: none;
      color: var(--text);
      background: var(--surface);
      border: 1px solid var(--border-strong);
      border-radius: 999px;
      padding: 10px 14px;
      font-size: 14px;
      transition: 0.18s ease;
    }}
    .nav-button:hover {{
      border-color: var(--accent);
      color: var(--accent);
    }}
    .nav-button.active {{
      background: var(--accent);
      color: #ffffff;
      border-color: var(--accent);
    }}
    section {{
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: 18px;
      padding: 22px;
      margin-bottom: 24px;
      box-shadow: var(--shadow);
    }}
    .hero-card {{
      padding: 28px 24px;
      background:
        radial-gradient(circle at top right, rgba(37, 99, 235, 0.16), transparent 30%),
        linear-gradient(180deg, #ffffff 0%, #f8fbff 100%);
    }}
    .eyebrow {{
      margin: 0 0 10px;
      color: var(--accent);
      font-weight: 700;
      font-size: 13px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }}
    h1, h2, h3 {{
      margin: 0 0 12px;
    }}
    .page-subtitle,
    .section-note,
    .chart-aim,
    .data-note {{
      color: var(--muted);
    }}
    .meta-row {{
      display: flex;
      flex-wrap: wrap;
      gap: 10px;
      margin-top: 16px;
    }}
    .meta-pill {{
      background: var(--accent-soft);
      border: 1px solid #bfdbfe;
      border-radius: 999px;
      padding: 8px 12px;
      font-size: 14px;
    }}
    .info-card {{
      background: var(--surface-soft);
      border-color: #bfdbfe;
    }}
    .formula {{
      font-family: "Courier New", monospace;
      background: #f8fafc;
      border: 1px solid var(--border-strong);
      border-radius: 10px;
      padding: 10px 12px;
      margin: 8px 0;
      display: inline-block;
    }}
    .link-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
      gap: 16px;
    }}
    .link-card {{
      display: block;
      text-decoration: none;
      color: var(--text);
      border: 1px solid var(--border-strong);
      border-radius: 16px;
      padding: 18px;
      background: linear-gradient(180deg, #ffffff 0%, #f8fbff 100%);
      transition: transform 0.18s ease, border-color 0.18s ease;
    }}
    .link-card:hover {{
      transform: translateY(-2px);
      border-color: var(--accent);
    }}
    .link-card strong {{
      display: block;
      margin-bottom: 6px;
    }}
    .link-card span {{
      color: var(--muted);
      font-size: 14px;
    }}
    table {{
      width: 100%;
      border-collapse: collapse;
      margin-top: 14px;
    }}
    th, td {{
      border: 1px solid var(--border-strong);
      padding: 10px 12px;
      text-align: left;
    }}
    th {{
      background: #e2e8f0;
    }}
    .chart-block {{
      background: var(--surface);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 16px;
    }}
    .chart-card {{
      border: 1px solid var(--border-strong);
      border-radius: 14px;
      padding: 12px;
      background: var(--surface);
      overflow-x: auto;
    }}
    ul {{
      margin-top: 8px;
      margin-bottom: 16px;
      padding-left: 20px;
    }}
    @media (max-width: 720px) {{
      .page {{
        width: min(100%, calc(100% - 20px));
        padding-top: 18px;
      }}
      .topbar-inner {{
        width: min(100%, calc(100% - 20px));
      }}
      section {{
        padding: 18px;
      }}
      .nav-buttons {{
        width: 100%;
      }}
      .nav-button {{
        flex: 1 1 150px;
        text-align: center;
      }}
    }}
  </style>
</head>
<body>
  <header class="topbar">
    <div class="topbar-inner">
      <div class="brand">Matrix Report</div>
      <nav class="nav-buttons">
        {self._build_nav_button("overview", active_page, "performance_report.html", "Overview")}
        {self._build_nav_button("sample", active_page, "sample_times.html", "Sample Times")}
        {self._build_nav_button("average", active_page, "average_time_comparison.html", "Average Time")}
        {self._build_nav_button("scaling", active_page, "why_speedup_is_not_linear.html", "Scaling")}
      </nav>
    </div>
  </header>
  <main class="page">
    {body}
  </main>
</body>
</html>
"""

    def _build_table_section(self, matrix_size: int, rows: list[ResultRow]) -> str:
        sample_counts = sorted({row.sample_count for row in rows if row.sample_count is not None})
        note = "Rows in this section use the same averaged run count."

        if len(sample_counts) == 1:
            note = f"Rows in this section are based on {sample_counts[0]} averaged runs."

        return f"""
<section>
  <h2>Matrix Size: {matrix_size} x {matrix_size}</h2>
  <p class="section-note">{html.escape(note)}</p>
  {self._build_table(rows)}
</section>
"""

    def _build_single_chart_section(
        self,
        matrix_size: int,
        rows: list[ResultRow],
        chart_title: str,
        aim_text: str,
        chart_html: str,
    ) -> str:
        sample_counts = sorted({row.sample_count for row in rows if row.sample_count is not None})
        note = "Rows in this section use the same averaged run count."

        if len(sample_counts) == 1:
            note = f"Rows in this section are based on {sample_counts[0]} averaged runs."

        return f"""
<section>
  <h2>Matrix Size: {matrix_size} x {matrix_size}</h2>
  <p class="section-note">{html.escape(note)}</p>
  <div class="chart-block">
    <h3>{html.escape(chart_title)}</h3>
    <p class="chart-aim">{html.escape(aim_text)}</p>
    <div class="chart-card">{chart_html}</div>
  </div>
</section>
"""

    @staticmethod
    def _build_nav_button(page_name: str, active_page: str, target: str, label: str) -> str:
        active_class = "nav-button active" if page_name == active_page else "nav-button"
        return f'<a class="{active_class}" href="{html.escape(target)}">{html.escape(label)}</a>'

    @staticmethod
    def _build_table(rows: list[ResultRow]) -> str:
        lines = [
            "<table>",
            "<thead>",
            "<tr>",
            "<th>Mode</th>",
            "<th>Threads</th>",
            "<th>Runs Averaged</th>",
            "<th>Average Time (s)</th>",
            "<th>Speedup</th>",
            "<th>Efficiency</th>",
            "<th>Verified</th>",
            "</tr>",
            "</thead>",
            "<tbody>",
        ]

        for row in rows:
            lines.extend(
                [
                    "<tr>",
                    f"<td>{html.escape(row.mode)}</td>",
                    f"<td>{row.threads}</td>",
                    f"<td>{ReportBuilder._format_optional_int(row.sample_count)}</td>",
                    f"<td>{row.execution_seconds:.6f}</td>",
                    f"<td>{ReportBuilder._format_number(row.speedup)}</td>",
                    f"<td>{ReportBuilder._format_number(row.efficiency)}</td>",
                    f"<td>{html.escape(row.verified or 'N/A')}</td>",
                    "</tr>",
                ]
            )

        lines.extend(["</tbody>", "</table>"])
        return "\n".join(lines)

    @staticmethod
    def _format_number(value: float | None, decimals: int = 4) -> str:
        if value is None:
            return "N/A"
        return f"{value:.{decimals}f}"

    @staticmethod
    def _format_optional_int(value: int | None) -> str:
        if value is None:
            return "N/A"
        return str(value)
