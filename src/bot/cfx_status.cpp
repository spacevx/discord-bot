#include "bot/cfx_status.hpp"
#include "bot/strings.hpp"

#include <functional>
#include <string>

namespace bot {

namespace {

constexpr auto RSS_URL = "https://status.cfx.re/history.rss";
constexpr std::size_t MAX_INCIDENTS = 5;

std::string extract_tag(const std::string& xml, std::string_view tag, std::size_t start, std::size_t end) {
    std::string open = std::string("<") + std::string(tag) + ">";
    std::string close = std::string("</") + std::string(tag) + ">";

    auto tag_start = xml.find(open, start);
    if (tag_start == std::string::npos || tag_start >= end) return {};

    tag_start += open.size();
    auto tag_end = xml.find(close, tag_start);
    if (tag_end == std::string::npos || tag_end > end) return {};

    return xml.substr(tag_start, tag_end - tag_start);
}

std::string strip_html(const std::string& html) {
    std::string result;
    result.reserve(html.size());
    bool in_tag = false;

    for (char c : html) {
        if (c == '<') { in_tag = true; continue; }
        if (c == '>') { in_tag = false; continue; }
        if (!in_tag) result += c;
    }

    return result;
}

std::string decode_entities(const std::string& s) {
    std::string out;
    out.reserve(s.size());

    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '&') {
            if (s.compare(i, 4, "&lt;") == 0) { out += '<'; i += 3; }
            else if (s.compare(i, 4, "&gt;") == 0) { out += '>'; i += 3; }
            else if (s.compare(i, 5, "&amp;") == 0) { out += '&'; i += 4; }
            else if (s.compare(i, 6, "&apos;") == 0) { out += '\''; i += 5; }
            else if (s.compare(i, 6, "&quot;") == 0) { out += '"'; i += 5; }
            else out += s[i];
        } else {
            out += s[i];
        }
    }

    return out;
}

std::string extract_first_strong(const std::string& decoded_desc) {
    auto start = decoded_desc.find("<strong>");
    if (start == std::string::npos) return {};
    start += 8;
    auto end = decoded_desc.find("</strong>", start);
    if (end == std::string::npos) return {};
    return decoded_desc.substr(start, end - start);
}

std::string to_lower(const std::string& s) {
    std::string lower;
    lower.reserve(s.size());
    for (char c : s) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower;
}

bool is_resolved(const std::string& status) {
    auto lower = to_lower(status);
    return lower == "resolved" || lower == "completed";
}

uint32_t color_from_status(const std::string& status) {
    auto lower = to_lower(status);

    if (lower == "resolved" || lower == "completed") return 0x2ECC71;
    if (lower == "monitoring" || lower == "verifying") return 0x3498DB;
    if (lower == "identified") return 0xE67E22;
    if (lower == "investigating") return 0xE74C3C;
    if (lower == "in progress" || lower == "scheduled") return 0xF1C40F;
    return 0x95A5A6;
}

void update_all_embeds(dpp::cluster& bot, const dpp::embed& embed) {
    auto channels = CfxStatus::instance().get_channels();

    for (auto& [channel_id, message_id] : channels) {
        dpp::message msg(dpp::snowflake(channel_id), embed);
        msg.id = message_id;
        bot.message_edit(msg);
    }
}

} // anonymous namespace

std::vector<CfxIncident> CfxStatus::parse_rss(const std::string& body) {
    std::vector<CfxIncident> incidents;

    std::size_t pos = 0;
    while (incidents.size() < MAX_INCIDENTS) {
        auto item_start = body.find("<item>", pos);
        if (item_start == std::string::npos) break;

        auto item_end = body.find("</item>", item_start);
        if (item_end == std::string::npos) break;

        item_end += 7;

        CfxIncident inc;
        inc.title = decode_entities(extract_tag(body, "title", item_start, item_end));

        auto raw_desc = decode_entities(extract_tag(body, "description", item_start, item_end));
        inc.status = extract_first_strong(raw_desc);
        inc.description = strip_html(raw_desc);

        inc.pub_date = extract_tag(body, "pubDate", item_start, item_end);
        inc.link = extract_tag(body, "link", item_start, item_end);

        if (!inc.title.empty()) {
            incidents.push_back(std::move(inc));
        }

        pos = item_end;
    }

    return incidents;
}

dpp::embed CfxStatus::build_embed(const std::vector<CfxIncident>& incidents) {
    namespace s = strings::cmd::cfxstatus;

    dpp::embed embed;
    embed.set_title(s::embed_title);
    embed.set_timestamp(time(nullptr));
    embed.set_footer(dpp::embed_footer().set_text(s::embed_footer));

    if (incidents.empty()) {
        embed.set_color(0x2ECC71);
        embed.set_description(s::no_incidents);
        return embed;
    }

    embed.set_color(color_from_status(incidents.front().status));

    for (const auto& inc : incidents) {
        std::string emoji = is_resolved(inc.status) ? s::status_up : s::status_down;

        std::string value = inc.description.empty() ? inc.pub_date
            : inc.description.substr(0, 200) + "\n" + inc.pub_date;

        if (!inc.link.empty()) {
            value += "\n[Details](" + inc.link + ")";
        }

        embed.add_field(emoji + " " + inc.title, value, false);
    }

    return embed;
}

void CfxStatus::start_polling(dpp::cluster& bot) {
    bot.start_timer([&bot](dpp::timer) {
        bot.request(RSS_URL, dpp::m_get, [&bot](const dpp::http_request_completion_t& response) {
            if (response.status != 200) return;

            auto& mgr = CfxStatus::instance();
            std::size_t hash = std::hash<std::string>{}(response.body);

            if (hash == mgr.get_last_hash()) return;
            mgr.set_last_hash(hash);

            auto incidents = parse_rss(response.body);
            auto embed = build_embed(incidents);
            update_all_embeds(bot, embed);
        });
    }, 60);
}

} // namespace bot
