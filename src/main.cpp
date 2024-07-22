#include "App.h"
#include "imgui.h"
#include "implot.h"
#include "audio.hpp"
#include "PitchDetector.hpp"
#include "ETFE.hpp"

// Buffer size and sample rate
static const int N = 8'192;
static const double fs = 8'000.0;

// Filter parameters
static double fc = 350.0;

namespace ImGui {

bool SliderDouble(const char *label, double *v, double v_min, double v_max, const char *format = NULL, ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
}

bool SliderInt32(const char *label, int *v, int v_min, int v_max, const char *format = NULL, ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

float GetWindowContentRegionWidth()
{
    return GetWindowContentRegionMax().x - GetWindowContentRegionMin().x;
}

} // namespace ImGui

struct ImFilter : public App {

    using App::App;

    void Update() override
    {
        constexpr double min_fc = 5.0;
        constexpr double max_fc = 500.0;
        // static note_table_t notes(fs, N);

        static PitchDetector pitch_detect;
        PitchDetector::Result note_res;

        // gui inputs
        static bool etfe_need_update = true;
        static bool filter_need_update = false;

        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(GetWindowSize(), ImGuiCond_Always);
        ImGui::Begin("Filter", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
        ImGui::BeginChild("ChildL", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, -1));

        ImGui::Text("Filter parameters");
        if (ImGui::SliderDouble("Cutoff frequency", &fc, min_fc, max_fc, "%.3f"))
            filter_need_update = true;
        ImGui::Separator();

        if (filter_need_update) {
            fc = std::clamp(fc, min_fc, max_fc);
            audio::setup_filter(fs, fc);
        }

        const audio::frames_t *frames = audio::get_buffer();

        // static Autocorrelation autocorr(N, fs);
        // const float *res = autocorr.apply(frames->y);
        // float freq = autocorr.get_freq();
        // note_res = pitch_detect.find(freq, 0.5);

        // ImGui::Text("Frequency: %.2f", freq);
        // ImGui::Text("Note: %s | %.2f", note_res.nearest_note->name, note_res.nearest_note->pitch);
        // if (note_res.in_tune) {
        //     ImGui::Text("In tune!");
        // } else {
        //     ImGui::Text("Cents sharp: %.2f", note_res.cents_sharp);
        // }

        // plot waveforms
        if (ImPlot::BeginPlot("##Filter", ImVec2(-1, -1))) {
            ImPlot::SetupAxes("Buffer Frames","Signal");
            ImPlot::SetupAxesLimits(0, N, -1, 1);

            ImPlot::SetupLegend(ImPlotLocation_NorthEast);
            ImPlot::PlotLine("Input Signal", frames->x, N);
            ImPlot::PlotLine("Filtered Signal", frames->y, N);
            // ImPlot::PlotLine("Autocorrelation", res, N);
            ImPlot::EndPlot();
        }
        ImGui::EndChild(); // ChildL
        ImGui::SameLine();
        ImGui::BeginChild("ChildR", ImVec2(0, -1));

        // perform ETFE
        static int window         = 0;
        static int inwindow       = 3;
        static int nwindow_opts[] = {100, 200, 500, 1'000, 2'000, 5'000, 10'000};
        static int infft          = 6;
        static int nfft_opts[]    = {100, 200, 500, 1'000, 2'000, 5'000, 10'000, 20'000, 50'000};
        static float overlap      = 0.5f;

        static etfe::ETFE etfe(N, fs, etfe::hamming(nwindow_opts[inwindow]), nwindow_opts[inwindow] / 2, nfft_opts[infft]);

        ImGui::Text("Frequency Response");
        ImGui::Separator();
        if (ImGui::Combo("FFT Size", &infft, "100\0""200\0""500\0""1000\0""2000\0""5000\0""10000\0""20000\0""50000\0")) {
            inwindow = infft < inwindow ? infft : inwindow;
            etfe_need_update = true;
        }

        if (ImGui::Combo("Window Type", &window, "hamming\0hann\0winrect\0"))
            etfe_need_update = true;
        if (ImGui::Combo("Window Size", &inwindow,"100\0""200\0""500\0""1000\0""2000\0""5000\0""10000\0"))
            etfe_need_update = true;
        if (ImGui::SliderFloat("Window Overlap", &overlap, 0, 1, "%.2f"))
            etfe_need_update = true;

        ImGui::NewLine();

        if (etfe_need_update) {
            infft = inwindow > infft ? inwindow : infft;
            int nwindow  = nwindow_opts[inwindow];
            int noverlap = (int)(nwindow * overlap);
            int nfft     = nfft_opts[infft];
            etfe.setup(
                N, fs,
                window == 0 ? etfe::hamming(nwindow)
                    : window == 1 ? etfe::hann(nwindow)
                    : etfe::winrect(nwindow),
                noverlap, nfft
            );
            etfe_need_update = false;
        }

        auto& fft_res = etfe.estimate(frames->y);
        note_res = pitch_detect.find(fft_res.f[fft_res.pidx], fft_res.df);

        if (ImGui::BeginTable("table", 4)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("Current frequency");
            ImGui::TableNextColumn();
            ImGui::Text("Nearest note frequency");
            ImGui::TableNextColumn();
            ImGui::Text("Nearest note name");
            ImGui::TableNextColumn();
            ImGui::Text("Accuracy");
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", fft_res.f[fft_res.pidx]);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", note_res.f);
            ImGui::TableNextColumn();
            ImGui::Text("%d%s", note_res.octave, note_res.note);
            ImGui::TableNextColumn();
            ImGui::Text("%.2f", fft_res.df);
            ImGui::EndTable();
        }
        if (note_res.in_tune) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
            ImGui::Text("In tune!");
            ImGui::PopStyleColor();
        } else {
            if (note_res.cents > 0) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                ImGui::Text("Cents sharp: %.2f", note_res.cents);
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                ImGui::Text("Cents flat: %.2f", -note_res.cents);
                ImGui::PopStyleColor();
            }
        }
        ImGui::Separator();

        if (ImPlot::BeginPlot("##Amp", ImVec2(-1, -1))) {
            ImPlot::SetupAxesLimits(0, 355, 0, 0.05);
            ImPlot::SetupAxes("Frequency [Hz]", "Amplitude [dB]");
            ImPlot::SetupLegend(ImPlotLocation_NorthEast);
            ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.25f);
            ImPlot::PlotShaded("x[f]", fft_res.f.data(), fft_res.ampy.data(), (int)fft_res.f.size(), -INFINITY);
            ImPlot::PlotLine("x[f]", fft_res.f.data(), fft_res.ampy.data(), (int)fft_res.f.size());
            ImPlot::TagX(fft_res.f[fft_res.pidx], ImVec4(1,1,1,0.9), "%s", note_res.note);
            if (ImPlot::DragLineX(397391, &fc, ImVec4(.15f, .15f, .15f, 1))) {
                filter_need_update = true;
            }
            ImPlot::EndPlot();
        }

        ImGui::EndChild();
        ImGui::End();
    }
};

int main(int argc, char const *argv[])
{
    audio::init(N, fs, fc);
    ImFilter app("ImFilter",960,540,argc,argv);
    app.Run();
    audio::deinit();

    return 0;
}
