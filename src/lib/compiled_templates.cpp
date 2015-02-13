#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
#include "controller/ban.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
	struct ban :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		Content::Ban &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		ban(std::ostream &_s,Content::Ban &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		virtual void render() {
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"css/ban.css\">\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 53 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body>\n"
				"        ";
			#line 53 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			settings();
			#line 55 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 55 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			navbar();
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"        <center><h1>";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banMessage);
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"</h1></center>\n"
				"        <div class=\"ban\">\n"
				"            <table>\n"
				"                <tr>\n"
				"                    <td>\n"
				"                        <b>";
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banBoardLabel);
			#line 64 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<":</b>\n"
				"                    </td>\n"
				"                    <td>\n"
				"                        ";
			#line 64 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banBoard);
			#line 69 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"                    </td>\n"
				"                </tr>\n"
				"                <tr>\n"
				"                    <td>\n"
				"                        <b>";
			#line 69 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banLevelLabel);
			#line 72 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<":</b>\n"
				"                    </td>\n"
				"                    <td>\n"
				"                        ";
			#line 72 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banLevel);
			#line 77 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"                    </td>\n"
				"                </tr>\n"
				"                <tr>\n"
				"                    <td>\n"
				"                        <b>";
			#line 77 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banDateTimeLabel);
			#line 80 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<":</b>\n"
				"                    </td>\n"
				"                    <td>\n"
				"                        ";
			#line 80 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<cppcms::filters::escape(content.banDateTime);
			#line 83 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"                    </td>\n"
				"                </tr>\n"
				"                ";
			#line 83 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			if( !content.banReason.empty() ) {
				#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"                    <tr>\n"
					"                        <td>\n"
					"                            <b>";
				#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<cppcms::filters::escape(content.banReasonLabel);
				#line 89 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<":</b>\n"
					"                        </td>\n"
					"                        <td>\n"
					"                            ";
				#line 89 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<cppcms::filters::escape(content.banReason);
				#line 92 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"                        </td>\n"
					"                    </tr>\n"
					"                ";
			#line 92 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			} // endif
			#line 93 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"                ";
			#line 93 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			if( !content.banExpires.empty() ) {
				#line 96 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"                    <tr>\n"
					"                        <td>\n"
					"                            <b>";
				#line 96 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<cppcms::filters::escape(content.banExpiresLabel);
				#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<":</b>\n"
					"                        </td>\n"
					"                        <td>\n"
					"                            ";
				#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<cppcms::filters::escape(content.banExpires);
				#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
				out()<<"\n"
					"                        </td>\n"
					"                    </tr>\n"
					"                ";
			#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			} // endif
			#line 107 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
			out()<<"\n"
				"            </table>\n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 107 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
		} // end of template render
	#line 109 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
	}; // end of class ban
#line 110 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/ban.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
#include "controller/board.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
	struct board :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		Content::Board &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		board(std::ostream &_s,Content::Board &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void banner() {
			#line 40 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"";
			#line 40 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( !content.bannerFileName.empty() ) {
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    <div class=\"banner\">\n"
					"        <img src=\"/";
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"img/banner/";
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.bannerFileName);
				#line 44 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">\n"
					"    </div>\n"
					"";
			#line 44 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"";
		#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template banner
		#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void createThreadAction(std::string position) {
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<form id=\"threadForm";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" class=\"postFormInvisible\" action=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"action/create_thread\"\n"
				"      method=\"post\" enctype=\"multipart/form-data\" accept-charset=\"utf-8\">\n"
				"    <input type=\"hidden\" id=\"threadFormInputBoard";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" value=\"";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.currentBoard.name);
			#line 55 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" name=\"board\" />\n"
				"    <table class=\"postFormTable\">\n"
				"        <tbody>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 55 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelEmail);
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input id=\"threadFormInputEmail";
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" type=\"text\" maxlength=\"150\" name=\"email\"\n"
				"                           size=\"30\" />\n"
				"                    <input type=\"submit\" value=\"";
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormButtonSubmit);
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelName);
			#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"text\" maxlength=\"50\" id=\"threadFormInputName";
			#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 73 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" name=\"name\" size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 73 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelSubject);
			#line 76 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"text\" maxlength=\"150\" id=\"threadFormInputSubject";
			#line 76 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 82 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" name=\"subject\"\n"
				"                           size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 82 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputText);
			#line 85 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <textarea name=\"text\" id=\"threadFormInputText";
			#line 85 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" rows=\"10\" cols=\"43\"\n"
				"                              placeholder=\"";
			#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputTextPlaceholder);
			#line 91 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" class=\"postFormTextarea\"></textarea>\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 91 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputFile);
			#line 94 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"file\" name=\"image\" id=\"threadFormInputFile";
			#line 94 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" accept=\"image/*\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelPassword);
			#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"password\" maxlength=\"150\" id=\"threadFormInputPassword";
			#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 106 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" name=\"password\"\n"
				"                           size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            ";
			#line 106 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.captchaEnabled ) {
				#line 109 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                <tr>\n"
					"                    <td class=\"postformLabel\">\n"
					"                        <b>";
				#line 109 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.postFormLabelCaptcha);
				#line 111 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</b>\n"
					"                    </td>\n"
					"                    <td id=\"googleCaptcha";
				#line 111 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 112 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\" class=\"postformField\">\n"
					"                        ";
				#line 112 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				if( "Top" == position ) {
					#line 113 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"                            <div id=\"googleCaptcha\" class=\"g-recaptcha\" data-sitekey=\"";
					#line 113 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.captchaKey);
					#line 114 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\"></div>\n"
						"                        ";
				#line 114 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // endif
				#line 117 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                    </td>\n"
					"                </tr>\n"
					"            ";
			#line 117 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 121 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        </tbody>\n"
				"    </table>\n"
				"</form>\n"
				"";
		#line 121 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template createThreadAction
		#line 123 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void searchAction(std::string position) {
			#line 124 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<form id=\"searchForm";
			#line 124 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 127 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" class=\"searchFormInvisible\">\n"
				"    <input type=\"text\" />\n"
				"</form>\n"
				"";
		#line 127 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template searchAction
		#line 129 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void actions(std::string position) {
			#line 131 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<div class=\"actions\">\n"
				"    <input type=\"hidden\" id=\"hideSearchFormText\" value=\"";
			#line 131 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.hideSearchFormText);
			#line 132 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"hideThreadFormText\" value=\"";
			#line 132 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.hideThreadFormText);
			#line 133 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"showSearchFormText\" value=\"";
			#line 133 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.showSearchFormText);
			#line 134 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"showThreadFormText\" value=\"";
			#line 134 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.showThreadFormText);
			#line 136 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\" />\n"
				"    <div>\n"
				"        ";
			#line 136 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.postingEnabled ) {
				#line 137 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            [<a id=\"showHideThreadFormButton";
				#line 137 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 138 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\"\n"
					"                 href=\"javascript:showHideThreadForm('";
				#line 138 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 139 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"');\">\n"
					"                ";
				#line 139 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.showThreadFormText);
				#line 141 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            </a>]\n"
					"        ";
			#line 141 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 142 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
			#line 142 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 143 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        [<a id=\"showHideSearchFormButton";
			#line 143 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 144 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\"\n"
				"            href=\"javascript:showHideSearchForm('";
			#line 144 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 145 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"');\">\n"
				"            ";
			#line 145 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.showSearchFormText);
			#line 148 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        </a>]\n"
				"    </div>\n"
				"    ";
			#line 148 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			searchAction(position);
			#line 149 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    ";
			#line 149 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.postingEnabled ) {
				#line 150 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
				#line 150 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				createThreadAction(position);
				#line 151 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 151 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 152 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 152 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 154 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"</div>\n"
				"";
		#line 154 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template actions
		#line 156 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void post(HelperPost post,HelperThread thread) {
			#line 157 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"";
			#line 157 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number ) {
				#line 159 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    <div class=\"opPost\">\n"
					"";
			#line 159 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 161 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    <div class=\"post\">\n"
					"";
			#line 161 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 163 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    <div class=\"postHeader\">\n"
				"        ";
			#line 163 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number && thread.fixed ) {
				#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <img src=\"/";
				#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"img/pin.png\" title=\"";
				#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.fixedText);
				#line 165 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">\n"
					"        ";
			#line 165 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number && !thread.postingEnabled ) {
				#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <img src=\"/";
				#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"img/closed.png\" title=\"";
				#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.closedText);
				#line 168 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">\n"
					"        ";
			#line 168 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <b>";
			#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(post.subject);
			#line 170 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</b>\n"
				"        ";
			#line 170 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( !post.email.empty() ) {
				#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <a href=\"mailto:";
				#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.email);
				#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">";
				#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.name);
				#line 172 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</a>\n"
					"        ";
			#line 172 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            ";
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.name);
				#line 174 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
			#line 174 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 175 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 175 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(post.dateTime);
			#line 176 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 176 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.postingEnabled ) {
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            #<a href=\"/";
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.name);
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"/thread/";
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(thread.opPost.number);
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<".html#i";
				#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 178 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">\n"
					"                ";
				#line 178 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 180 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            </a>\n"
					"        ";
			#line 180 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            #<a href=\"/";
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.name);
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"/thread/";
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(thread.opPost.number);
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<".html#";
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 182 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\">\n"
					"                ";
				#line 182 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 184 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            </a>\n"
					"        ";
			#line 184 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 185 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 185 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number ) {
				#line 186 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            [<a href=\"/";
				#line 186 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 186 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.name);
				#line 186 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"/thread/";
				#line 186 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 187 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<".html\">\n"
					"                ";
				#line 187 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.toThread);
				#line 189 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            </a>]\n"
					"        ";
			#line 189 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 190 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 190 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number && thread.postLimitReached() ) {
				#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <span class=\"postLimitReached\">";
				#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.postLimitReachedText);
				#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</span>\n"
					"        ";
			#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			else
			#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.number == thread.opPost.number && thread.bumpLimitReached() ) {
				#line 193 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <span class=\"bumpLimitReached\">";
				#line 193 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.bumpLimitReachedText);
				#line 194 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</span>\n"
					"        ";
			#line 194 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    </div>\n"
				"    <table class=\"threadPosts\">\n"
				"        <tbody>\n"
				"            <tr>\n"
				"                ";
			#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if((post.files).begin()!=(post.files).end()) {
				#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                    ";
				#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				for(CPPCMS_TYPEOF((post.files).begin()) file_ptr=(post.files).begin(),file_ptr_end=(post.files).end();file_ptr!=file_ptr_end;++file_ptr) {
				#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				CPPCMS_TYPEOF(*file_ptr) &file=*file_ptr;
					#line 203 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"                        <td class=\"postFile\">\n"
						"                            <div>\n"
						"                                <a href=\"/";
					#line 203 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 203 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 203 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"/";
					#line 203 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 204 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\">\n"
						"                                    ";
					#line 204 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 208 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"                                </a>\n"
						"                            </div class=\"postFileName\">\n"
						"                            <div class=\"postFileSize\">\n"
						"                                (";
					#line 208 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(file.size);
					#line 211 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<")\n"
						"                            </div>\n"
						"                            <div>\n"
						"                                <a href=\"/";
					#line 211 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 211 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 211 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"/";
					#line 211 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 212 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\">\n"
						"                                    <img src=\"/";
					#line 212 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 212 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 212 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"/";
					#line 212 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(file.thumbName);
					#line 216 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\">\n"
						"                                </a>\n"
						"                            </div>\n"
						"                        </td>\n"
						"                    ";
				#line 216 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // end of item
				#line 217 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                ";
			#line 217 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			 } else {
				#line 218 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                ";
			#line 218 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // end of empty
			#line 220 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"                <td class=\"postText\">\n"
				"                    <blockquote>";
			#line 220 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::raw(post.text);
			#line 225 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</blockquote>\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td colspan=\"2\">\n"
				"                    ";
			#line 225 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( post.bannedFor ) {
				#line 226 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"                        <span class=\"bannedFor\">";
				#line 226 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.bannedForText);
				#line 227 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</span>\n"
					"                    ";
			#line 227 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 233 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"                <td>\n"
				"            </tr>\n"
				"        </tbody>\n"
				"    </table>\n"
				"</div>\n"
				"";
		#line 233 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template post
		#line 235 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void threads() {
			#line 236 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"";
			#line 236 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if((content.threads).begin()!=(content.threads).end()) {
				#line 237 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
				#line 237 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				for(CPPCMS_TYPEOF((content.threads).begin()) thread_ptr=(content.threads).begin(),thread_ptr_end=(content.threads).end();thread_ptr!=thread_ptr_end;++thread_ptr) {
				#line 237 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				CPPCMS_TYPEOF(*thread_ptr) &thread=*thread_ptr;
					#line 240 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"        <hr />\n"
						"        <div>\n"
						"            ";
					#line 240 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					post(thread.opPost,thread);
					#line 242 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"        </div>\n"
						"        ";
					#line 242 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					if( thread.omittedPosts() > 0 ) {
						#line 244 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"            <div>\n"
							"                ";
						#line 244 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<cppcms::filters::escape(content.omittedPostsText);
						#line 244 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<" ";
						#line 244 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<cppcms::filters::escape(thread.omittedPosts());
						#line 246 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"            </div>\n"
							"        ";
					#line 246 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}else{
						#line 247 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"        ";
					#line 247 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}
					#line 248 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"        ";
					#line 248 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					if((thread.lastPosts).begin()!=(thread.lastPosts).end()) {
						#line 249 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"            ";
						#line 249 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						for(CPPCMS_TYPEOF((thread.lastPosts).begin()) p_ptr=(thread.lastPosts).begin(),p_ptr_end=(thread.lastPosts).end();p_ptr!=p_ptr_end;++p_ptr) {
						#line 249 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						CPPCMS_TYPEOF(*p_ptr) &p=*p_ptr;
							#line 251 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"\n"
								"                <div>\n"
								"                    ";
							#line 251 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							post(p,thread);
							#line 253 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"\n"
								"                </div>\n"
								"            ";
						#line 253 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						} // end of item
						#line 254 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"        ";
					#line 254 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}
					#line 255 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"    ";
				#line 255 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // end of item
				#line 256 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"";
			#line 256 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			 } else {
				#line 257 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"";
			#line 257 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // end of empty
			#line 258 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"";
		#line 258 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template threads
		#line 260 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void pages() {
			#line 262 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<div class=\"pages\">\n"
				"    ";
			#line 262 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.currentPage > 0 ) {
				#line 264 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        <span class=\"pagesItem\">\n"
					"        ";
				#line 264 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				if( 1 == content.currentPage ) {
					#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"            [<a href=\"/";
					#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\">";
					#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.toPreviousPageText);
					#line 266 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"</a>]\n"
						"        ";
				#line 266 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				}else{
					#line 267 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"            [<a href=\"/";
					#line 267 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 267 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 267 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"/";
					#line 267 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.previousPage());
					#line 268 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<".html\">\n"
						"                ";
					#line 268 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<cppcms::filters::escape(content.toPreviousPageText);
					#line 270 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"            </a>] \n"
						"        ";
				#line 270 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				}
				#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        </span>\n"
					"    ";
			#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 273 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    ";
			#line 273 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if((content.pages).begin()!=(content.pages).end()) {
				#line 274 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
				#line 274 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				for(CPPCMS_TYPEOF((content.pages).begin()) page_ptr=(content.pages).begin(),page_ptr_end=(content.pages).end();page_ptr!=page_ptr_end;++page_ptr) {
				#line 274 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				CPPCMS_TYPEOF(*page_ptr) &page=*page_ptr;
					#line 276 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"            <span class=\"pagesItem\">\n"
						"            ";
					#line 276 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					if( page != content.currentPage ) {
						#line 277 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"                ";
						#line 277 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						if( page > 0 ) {
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"\n"
								"                    [<a href=\"/";
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(content.sitePathPrefix);
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(content.currentBoard.name);
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"/";
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(page);
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<".html\">";
							#line 278 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(page);
							#line 279 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"</a>]\n"
								"                ";
						#line 279 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						}else{
							#line 280 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"\n"
								"                    [<a href=\"/";
							#line 280 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(content.sitePathPrefix);
							#line 280 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(content.currentBoard.name);
							#line 280 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"\">";
							#line 280 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<cppcms::filters::escape(page);
							#line 281 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
							out()<<"</a>]\n"
								"                ";
						#line 281 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						}
						#line 282 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"            ";
					#line 282 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}else{
						#line 283 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"\n"
							"                [<b>";
						#line 283 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<cppcms::filters::escape(page);
						#line 284 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
						out()<<"</b>]\n"
							"            ";
					#line 284 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					}
					#line 286 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
					out()<<"\n"
						"            </span>\n"
						"        ";
				#line 286 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				} // end of item
				#line 287 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 287 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			 } else {
				#line 288 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"    ";
			#line 288 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // end of empty
			#line 289 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    ";
			#line 289 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.currentPage < content.pages.size() - 1 ) {
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        <span class=\"pagesItem\">\n"
					"            [<a href=\"/";
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.name);
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"/";
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.nextPage());
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<".html\">";
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.toNextPageText);
				#line 293 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</a>]\n"
					"        </span>\n"
					"    ";
			#line 293 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 295 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"</div>\n"
				"";
		#line 295 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template pages
		#line 297 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		virtual void render() {
			#line 301 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 301 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 303 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 303 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 304 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 304 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 305 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 305 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 306 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 306 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 307 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 307 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 308 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/banner.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 308 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 309 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/postform.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 309 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 310 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/searchform.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 310 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 311 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/actions.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 311 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 312 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/post.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 312 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 313 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 313 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 316 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"css/board.css\">\n"
				"        <!--script src=\"http://code.jquery.com/jquery-2.1.3.min.js\"></script-->\n"
				"        <!--script src=\"http://ajax.aspnetcdn.com/ajax/jquery.validate/1.13.1/jquery.validate.min.js\"></script-->\n"
				"        ";
			#line 316 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( content.captchaEnabled ) {
				#line 318 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <script type=\"text/javascript\" src=\"https://www.google.com/recaptcha/api.js\"></script>\n"
					"        ";
			#line 318 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 319 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <!--script src=\"/";
			#line 319 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 320 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"js/3rdparty/sha256.js\"></script-->\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 320 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 321 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"js/board.js\"></script>\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 321 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 324 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body onload=\"javascript:initializeOnLoad();\">\n"
				"        ";
			#line 324 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			settings();
			#line 326 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 326 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			navbar();
			#line 327 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 327 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( !content.bannerFileName.empty() ) {
				#line 329 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <br />\n"
					"        ";
			#line 329 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			} // endif
			#line 330 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 330 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			banner();
			#line 333 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            <h1>\n"
				"                ";
			#line 333 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.currentBoard.title);
			#line 334 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"                <sup>[<a href=\"/";
			#line 334 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 334 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.currentBoard.name);
			#line 335 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"/rules.html\"\n"
				"                         title=\"";
			#line 335 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<cppcms::filters::escape(content.boardRulesLinkText);
			#line 338 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\">?</a>]</sup>\n"
				"            </h1>\n"
				"        </div>\n"
				"        ";
			#line 338 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			if( !content.postingEnabled ) {
				#line 340 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"            <div class=\"postingDisabledMessage\">\n"
					"                <h2>";
				#line 340 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<cppcms::filters::escape(content.postingDisabledText);
				#line 342 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"</h2>\n"
					"            </div>\n"
					"        ";
			#line 342 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}else{
				#line 343 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
				out()<<"\n"
					"        ";
			#line 343 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			}
			#line 345 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 345 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			actions("Top");
			#line 346 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        ";
			#line 346 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			threads();
			#line 348 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 348 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			actions("Bottom");
			#line 350 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 350 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			pages();
			#line 352 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 352 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			navbar();
			#line 355 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
			out()<<"\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 355 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
		} // end of template render
	#line 357 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
	}; // end of class board
#line 358 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
#include "controller/board_image.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
	struct board_image :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		Content::BoardImage &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		board_image(std::ostream &_s,Content::BoardImage &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		virtual void render() {
			#line 9 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 9 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"css/board_image.css\">\n"
				"    </head>\n"
				"    <body>\n"
				"        <div class=\"theImage\">\n"
				"            <img src=\"/";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"img/";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.imageFileName);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"\" title=\"";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<cppcms::filters::escape(content.imageTitle);
			#line 21 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
			out()<<"\">\n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 21 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
		} // end of template render
	#line 23 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
	}; // end of class board_image
#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_image.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
#include "controller/board_video.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
	struct board_video :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		Content::BoardVideo &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		board_video(std::ostream &_s,Content::BoardVideo &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		virtual void render() {
			#line 9 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 9 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"css/base.css\">\n"
				"    </head>\n"
				"    <body>\n"
				"        <div class=\"theVideo\">\n"
				"            <video autoplay=\"on\" loop=\"on\">\n"
				"                <source src=\"/";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"video/";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.videoFileName);
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"\" type=\"";
			#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.videoType);
			#line 18 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"\" />\n"
				"                ";
			#line 18 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<cppcms::filters::escape(content.altVideoText);
			#line 23 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
			out()<<"\n"
				"            </video> \n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 23 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
		} // end of template render
	#line 25 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
	}; // end of class board_video
#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/board_video.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
#include "controller/error.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
	struct error :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		Content::Error &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		error(std::ostream &_s,Content::Error &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		virtual void render() {
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"css/error.css\">\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body>\n"
				"        ";
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			settings();
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			navbar();
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            <h1>";
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.errorMessage);
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"</h1>\n"
				"        </div>\n"
				"        <div class=\"error\">\n"
				"            ";
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<cppcms::filters::escape(content.errorDescription);
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
			out()<<"\n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
		} // end of template render
	#line 67 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
	}; // end of class error
#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/error.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
#include "controller/home.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
	struct home :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		Content::Home &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		home(std::ostream &_s,Content::Home &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		virtual void render() {
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"css/home.css\">\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body>\n"
				"        ";
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			settings();
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			navbar();
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            <h1>";
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<cppcms::filters::escape(content.welcomeMessage);
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"</h1>\n"
				"        </div>\n"
				"        ";
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			if( !content.rules.empty() ) {
				#line 63 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"            <div class=\"rules\">\n"
					"                <ol>\n"
					"                ";
				#line 63 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				if((content.rules).begin()!=(content.rules).end()) {
					#line 64 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\n"
						"                     ";
					#line 64 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					for(CPPCMS_TYPEOF((content.rules).begin()) rule_ptr=(content.rules).begin(),rule_ptr_end=(content.rules).end();rule_ptr!=rule_ptr_end;++rule_ptr) {
					#line 64 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					CPPCMS_TYPEOF(*rule_ptr) &rule=*rule_ptr;
						#line 66 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\n"
							"                        <li>\n"
							"                            ";
						#line 66 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<cppcms::filters::raw(rule);
						#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
						out()<<"\n"
							"                        </li>\n"
							"                    ";
					#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					} // end of item
					#line 69 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
					out()<<"\n"
						"                ";
				#line 69 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				}
				#line 72 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
				out()<<"\n"
					"                </ol>\n"
					"            </div>\n"
					"        ";
			#line 72 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			} // endif
			#line 75 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
			out()<<"\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 75 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
		} // end of template render
	#line 77 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
	}; // end of class home
#line 78 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/home.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
#include "controller/notfound.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
	struct not_found :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		Content::NotFound &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		not_found(std::ostream &_s,Content::NotFound &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		virtual void render() {
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"css/not_found.css\">\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body>\n"
				"        ";
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			settings();
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			navbar();
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            <h1>";
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.notFoundMessage);
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"</h1>\n"
				"        </div>\n"
				"        <div class=\"theImage\">\n"
				"            <img src=\"/";
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"img/";
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.imageFileName);
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\" title=\"";
			#line 61 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<cppcms::filters::escape(content.imageTitle);
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
			out()<<"\">\n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
		} // end of template render
	#line 67 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
	}; // end of class not_found
#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/not_found.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
#include "controller/rules.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
	struct rules :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		Content::Rules &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		rules(std::ostream &_s,Content::Rules &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		virtual void render() {
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        <title>";
			#line 43 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.pageTitle);
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"</title>\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 46 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 49 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"css/rules.css\">\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body>\n"
				"        ";
			#line 54 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			settings();
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			navbar();
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            <h1>";
			#line 58 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<cppcms::filters::escape(content.currentBoard.title);
			#line 62 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"</h1>\n"
				"        </div>\n"
				"        <div class=\"rules\">\n"
				"            <ol>\n"
				"            ";
			#line 62 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			if((content.rules).begin()!=(content.rules).end()) {
				#line 63 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"                ";
				#line 63 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				for(CPPCMS_TYPEOF((content.rules).begin()) rule_ptr=(content.rules).begin(),rule_ptr_end=(content.rules).end();rule_ptr!=rule_ptr_end;++rule_ptr) {
				#line 63 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				CPPCMS_TYPEOF(*rule_ptr) &rule=*rule_ptr;
					#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\n"
						"                    <li>\n"
						"                        ";
					#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<cppcms::filters::raw(rule);
					#line 67 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
					out()<<"\n"
						"                    </li>\n"
						"                ";
				#line 67 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				} // end of item
				#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"            ";
			#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			 } else {
				#line 71 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"                </ol>\n"
					"                <p>\n"
					"                    ";
				#line 71 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<cppcms::filters::escape(content.noRulesText);
				#line 74 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
				out()<<"\n"
					"                </p>\n"
					"                <ol>\n"
					"            ";
			#line 74 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			} // end of empty
			#line 79 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
			out()<<"\n"
				"            </ol>\n"
				"        </div>\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 79 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
		} // end of template render
	#line 81 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
	}; // end of class rules
#line 82 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/rules.tmpl"
} // end of namespace my_skin
#line 1 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
#include "controller/thread.h" 
#line 2 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
namespace my_skin {
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
	struct thread :public cppcms::base_view
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
	{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		Content::Thread &content;
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		thread(std::ostream &_s,Content::Thread &_content): cppcms::base_view(_s),content(_content)
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		{
	#line 3 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		}
		#line 5 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void settings() {
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<div class=\"settingsBar\">\n"
				"    <span>\n"
				"        ";
			#line 8 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.localeLabelText);
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <select id=\"localeChangeSelect\" class=\"select\" onchange=\"javascript:changeLocale();\">\n"
				"            ";
			#line 10 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if((content.locales).begin()!=(content.locales).end()) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                ";
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				for(CPPCMS_TYPEOF((content.locales).begin()) locale_ptr=(content.locales).begin(),locale_ptr_end=(content.locales).end();locale_ptr!=locale_ptr_end;++locale_ptr) {
				#line 11 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				CPPCMS_TYPEOF(*locale_ptr) &locale=*locale_ptr;
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                    ";
					#line 12 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					if( locale.name == content.currentLocale.name ) {
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 13 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<"\" selected=\"true\">\n"
							"                    ";
					#line 14 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					}else{
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<"\n"
							"                        <option value=\"";
						#line 15 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<cppcms::filters::escape(locale.name);
						#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
						out()<<"\">\n"
							"                    ";
					#line 16 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					}
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                        ";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(locale.language);
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<" (";
					#line 17 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(locale.country);
					#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<")\n"
						"                    </option>\n"
						"                ";
				#line 19 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				} // end of item
				#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            ";
			#line 20 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        </select>\n"
				"    </span>\n"
				"</div>\n"
				"";
		#line 24 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template settings
		#line 26 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void navbar() {
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<div class=\"navbar\">\n"
				"    ";
			#line 28 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if((content.boards).begin()!=(content.boards).end()) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				for(CPPCMS_TYPEOF((content.boards).begin()) board_ptr=(content.boards).begin(),board_ptr_end=(content.boards).end();board_ptr!=board_ptr_end;++board_ptr) {
				#line 29 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				CPPCMS_TYPEOF(*board_ptr) &board=*board_ptr;
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"        <span class=\"navbarItem\">[<a href=\"/";
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 30 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\"\n"
						"              title=\"";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(board.title);
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\">";
					#line 31 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(board.name);
					#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"</a>] </span>\n"
						"        ";
				#line 32 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				} // end of item
				#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    ";
			#line 33 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			 } else {
				#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    ";
			#line 34 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // end of empty
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"    <span class=\"navbarItem\">[<a href=\"/";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\">";
			#line 35 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.toHomePageText);
			#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</a>]</span>\n"
				"</div>\n"
				"";
		#line 37 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template navbar
		#line 39 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void banner() {
			#line 40 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"";
			#line 40 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.bannerFileName.empty() ) {
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    <div class=\"banner\">\n"
					"        <img src=\"/";
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"img/banner/";
				#line 42 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.bannerFileName);
				#line 44 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\">\n"
					"    </div>\n"
					"";
			#line 44 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"";
		#line 45 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template banner
		#line 47 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void createPostAction(std::string position) {
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<form id=\"postForm";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" class=\"postFormInvisible\" action=\"/";
			#line 48 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"action/create_post\"\n"
				"      method=\"post\" enctype=\"multipart/form-data\" accept-charset=\"utf-8\">\n"
				"    <input type=\"hidden\" id=\"postFormInputBoard";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" value=\"";
			#line 50 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.currentBoard.name);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" name=\"board\" />\n"
				"    <input type=\"hidden\" id=\"postFormInputThread";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" value=\"";
			#line 51 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.currentThread);
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" name=\"thread\" />\n"
				"    <table class=\"postFormTable\">\n"
				"        <tbody>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 56 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelEmail);
			#line 59 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input id=\"postFormInputEmail";
			#line 59 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" type=\"text\" maxlength=\"150\" name=\"email\" size=\"30\" />\n"
				"                    <input type=\"submit\" value=\"";
			#line 60 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormButtonSubmit);
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 65 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelName);
			#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"text\" maxlength=\"50\" id=\"postFormInputName";
			#line 68 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 73 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" name=\"name\" size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 73 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelSubject);
			#line 76 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"text\" maxlength=\"150\" id=\"postFormInputSubject";
			#line 76 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 82 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" name=\"subject\"\n"
				"                           size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 82 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputText);
			#line 85 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <textarea name=\"text\" id=\"postFormInputText";
			#line 85 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" rows=\"10\" cols=\"43\"\n"
				"                              placeholder=\"";
			#line 86 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputTextPlaceholder);
			#line 91 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" class=\"postFormTextarea\"></textarea>\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 91 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormInputFile);
			#line 94 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"file\" name=\"image\" id=\"postFormInputFile";
			#line 94 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" accept=\"image/*\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td class=\"postformLabel\">\n"
				"                    <b>";
			#line 99 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.postFormLabelPassword);
			#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"                </td>\n"
				"                <td class=\"postformField\">\n"
				"                    <input type=\"password\" maxlength=\"150\" id=\"postFormInputPassword";
			#line 102 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 106 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" name=\"password\"\n"
				"                           size=\"43\" />\n"
				"                </td>\n"
				"            </tr>\n"
				"            ";
			#line 106 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.captchaEnabled ) {
				#line 109 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                <tr>\n"
					"                    <td class=\"postformLabel\">\n"
					"                        <b>";
				#line 109 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.postFormLabelCaptcha);
				#line 111 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</b>\n"
					"                    </td>\n"
					"                    <td id=\"googleCaptcha";
				#line 111 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 112 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\" class=\"postformField\">\n"
					"                        ";
				#line 112 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				if( "Top" == position ) {
					#line 113 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                            <div id=\"googleCaptcha\" class=\"g-recaptcha\" data-sitekey=\"";
					#line 113 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.captchaKey);
					#line 114 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\"></div>\n"
						"                        ";
				#line 114 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				} // endif
				#line 117 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                    </td>\n"
					"                </tr>\n"
					"            ";
			#line 117 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 121 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        </tbody>\n"
				"    </table>\n"
				"</form>\n"
				"";
		#line 121 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template createPostAction
		#line 123 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void searchAction(std::string position) {
			#line 124 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<form id=\"searchForm";
			#line 124 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 127 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" class=\"searchFormInvisible\">\n"
				"    <input type=\"text\" />\n"
				"</form>\n"
				"";
		#line 127 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template searchAction
		#line 129 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void actions(std::string position) {
			#line 131 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<div class=\"actions\">\n"
				"    <input type=\"hidden\" id=\"hideSearchFormText\" value=\"";
			#line 131 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.hideSearchFormText);
			#line 132 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"hidePostFormText\" value=\"";
			#line 132 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.hidePostFormText);
			#line 133 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"showSearchFormText\" value=\"";
			#line 133 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.showSearchFormText);
			#line 134 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" />\n"
				"    <input type=\"hidden\" id=\"showPostFormText\" value=\"";
			#line 134 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.showPostFormText);
			#line 137 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\" />\n"
				"    <br />\n"
				"    <div>\n"
				"        ";
			#line 137 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.postingEnabled && !content.postLimitReached() ) {
				#line 138 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            [<a id=\"showHidePostFormButton";
				#line 138 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 139 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\"\n"
					"                href=\"javascript:showHidePostForm('";
				#line 139 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(position);
				#line 140 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"');\">\n"
					"                ";
				#line 140 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.showPostFormText);
				#line 142 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            </a>]\n"
					"        ";
			#line 142 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 143 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
			#line 143 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 144 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        [<a id=\"showHideSearchFormButton";
			#line 144 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 145 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\"\n"
				"            href=\"javascript:showHideSearchForm('";
			#line 145 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(position);
			#line 146 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"');\">\n"
				"            ";
			#line 146 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.showSearchFormText);
			#line 149 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        </a>]\n"
				"    </div>\n"
				"    ";
			#line 149 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			searchAction(position);
			#line 150 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"    ";
			#line 150 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.postingEnabled ) {
				#line 151 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
				#line 151 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				createPostAction(position);
				#line 152 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    ";
			#line 152 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 153 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    ";
			#line 153 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 155 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"</div>\n"
				"";
		#line 155 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template actions
		#line 157 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void post(HelperPost post,bool op,bool fixed) {
			#line 158 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"";
			#line 158 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( op ) {
				#line 159 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    <div id=\"post";
				#line 159 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 160 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\" class=\"opPost\">\n"
					"";
			#line 160 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 161 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"    <div id=\"post";
				#line 161 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 162 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\" class=\"post\">\n"
					"";
			#line 162 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"    <div class=\"postHeader\">\n"
				"        <a id=\"";
			#line 164 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(post.number);
			#line 165 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\"></a>\n"
				"        ";
			#line 165 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( op && fixed ) {
				#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <img src=\"/";
				#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"img/pin.png\" title=\"";
				#line 166 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.fixedText);
				#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\">\n"
					"        ";
			#line 167 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 168 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 168 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( op && !content.postingEnabled ) {
				#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <img src=\"/";
				#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.sitePathPrefix);
				#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"img/closed.png\" title=\"";
				#line 169 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.closedText);
				#line 170 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\">\n"
					"        ";
			#line 170 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <b>";
			#line 171 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(post.subject);
			#line 172 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</b>\n"
				"        ";
			#line 172 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !post.email.empty() ) {
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <a href=\"mailto:";
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.email);
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\">";
				#line 173 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.name);
				#line 174 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</a>\n"
					"        ";
			#line 174 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 175 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            ";
				#line 175 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.name);
				#line 176 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
			#line 176 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 177 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(post.dateTime);
			#line 178 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 178 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.postingEnabled ) {
				#line 179 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            #<a href=\"javascript:insertPostNumber('";
				#line 179 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 179 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"');\">";
				#line 179 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 180 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</a>\n"
					"        ";
			#line 180 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            #<a href=\"#";
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\">";
				#line 181 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(post.number);
				#line 182 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</a>\n"
					"        ";
			#line 182 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 187 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"    </div>\n"
				"    <table class=\"threadPosts\">\n"
				"        <tbody>\n"
				"            <tr>\n"
				"                ";
			#line 187 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if((post.files).begin()!=(post.files).end()) {
				#line 188 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                    ";
				#line 188 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				for(CPPCMS_TYPEOF((post.files).begin()) file_ptr=(post.files).begin(),file_ptr_end=(post.files).end();file_ptr!=file_ptr_end;++file_ptr) {
				#line 188 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				CPPCMS_TYPEOF(*file_ptr) &file=*file_ptr;
					#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                        <td class=\"postFile\">\n"
						"                            <div class=\"postFileName\">\n"
						"                                <a href=\"/";
					#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"/";
					#line 191 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\">\n"
						"                                    ";
					#line 192 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 196 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                                </a>\n"
						"                            </div>\n"
						"                            <div class=\"postFileSize\">\n"
						"                                (";
					#line 196 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(file.size);
					#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<")\n"
						"                            </div>\n"
						"                            <div>\n"
						"                                <a href=\"/";
					#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"/";
					#line 199 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(file.sourceName);
					#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\">\n"
						"                                    <img src=\"/";
					#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.sitePathPrefix);
					#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(content.currentBoard.name);
					#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"/";
					#line 200 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<cppcms::filters::escape(file.thumbName);
					#line 204 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\">\n"
						"                                </a>\n"
						"                            </div>\n"
						"                        </td>\n"
						"                    ";
				#line 204 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				} // end of item
				#line 205 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                ";
			#line 205 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			 } else {
				#line 206 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                ";
			#line 206 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // end of empty
			#line 208 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"                <td class=\"postText\">\n"
				"                    <blockquote>";
			#line 208 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::raw(post.text);
			#line 213 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"</blockquote>\n"
				"                </td>\n"
				"            </tr>\n"
				"            <tr>\n"
				"                <td colspan=\"2\">\n"
				"                    ";
			#line 213 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( post.bannedFor ) {
				#line 214 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                        <span class=\"bannedFor\">";
				#line 214 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.bannedForText);
				#line 215 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</span>\n"
					"                    ";
			#line 215 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 221 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"                <td>\n"
				"            </tr>\n"
				"        </tbody>\n"
				"    </table>\n"
				"</div>\n"
				"";
		#line 221 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template post
		#line 223 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		virtual void render() {
			#line 227 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"<!DOCTYPE html>\n"
				"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
				"    <head>\n"
				"        ";
			#line 227 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.pageTitle.empty() ) {
				#line 228 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <title>";
				#line 228 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.pageTitle);
				#line 229 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</title>\n"
					"        ";
			#line 229 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 230 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <title>";
				#line 230 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.title);
				#line 230 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<" - ";
				#line 230 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.currentThread);
				#line 231 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</title>\n"
					"        ";
			#line 231 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 233 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
				"        <link id=\"favicon\" rel=\"shortcut icon\" href=\"/";
			#line 233 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 234 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"favicon.ico\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 234 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 235 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/base.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 235 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 236 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/settings_bar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 236 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 237 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/navbar.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 237 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 238 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/banner.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 238 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 239 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/postform.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 239 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 240 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/searchform.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 240 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 241 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/actions.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 241 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 242 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/post.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 242 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 243 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/title.css\">\n"
				"        <link rel=\"stylesheet\" type=\"text/css\" href=\"/";
			#line 243 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 246 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"css/thread.css\">\n"
				"        <!--script src=\"http://code.jquery.com/jquery-2.1.3.min.js\"></script-->\n"
				"        <!--script src=\"http://ajax.aspnetcdn.com/ajax/jquery.validate/1.13.1/jquery.validate.min.js\"></script-->\n"
				"        ";
			#line 246 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.captchaEnabled ) {
				#line 248 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <script type=\"text/javascript\" src=\"https://www.google.com/recaptcha/api.js\"></script>\n"
					"        ";
			#line 248 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 249 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <!--script src=\"/";
			#line 249 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 250 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"js/3rdparty/sha256.js\"></script-->\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 250 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 251 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"js/thread.js\"></script>\n"
				"        <script type=\"text/javascript\" src=\"/";
			#line 251 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<cppcms::filters::escape(content.sitePathPrefix);
			#line 254 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"js/settings.js\"></script>\n"
				"    </head>\n"
				"    <body onload=\"javascript:initializeOnLoad();\">\n"
				"        ";
			#line 254 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			settings();
			#line 256 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <br />\n"
				"        ";
			#line 256 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			navbar();
			#line 257 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 257 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.bannerFileName.empty() ) {
				#line 259 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <br />\n"
					"        ";
			#line 259 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 260 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 260 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			banner();
			#line 262 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <div class=\"theTitle\">\n"
				"            ";
			#line 262 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.pageTitle.empty() ) {
				#line 263 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                <h1>";
				#line 263 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.pageTitle);
				#line 264 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</h1>\n"
					"            ";
			#line 264 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}else{
				#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"                <h1>";
				#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.currentBoard.title);
				#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<" - ";
				#line 265 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.currentThread);
				#line 266 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</h1>\n"
					"            ";
			#line 266 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 268 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        </div>\n"
				"        ";
			#line 268 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.postingEnabled ) {
				#line 270 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <div class=\"theMessage\">\n"
					"                <h2>";
				#line 270 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.postingDisabledText);
				#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</h2>\n"
					"            </div>\n"
					"        ";
			#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			}
			#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			else
			#line 272 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( content.postLimitReached() ) {
				#line 274 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <div class=\"theMessage\">\n"
					"                <h2>";
				#line 274 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.postLimitReachedText);
				#line 276 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</h2>\n"
					"            </div>\n"
					"        ";
			#line 276 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 277 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 277 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if( !content.postLimitReached() && content.bumpLimitReached() ) {
				#line 279 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            <div class=\"theMessage\">\n"
					"                <h3>";
				#line 279 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<cppcms::filters::escape(content.bumpLimitReachedText);
				#line 281 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"</h3>\n"
					"            </div>\n"
					"        ";
			#line 281 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // endif
			#line 283 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 283 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			actions("Top");
			#line 285 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 285 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			post(content.opPost,1,content.fixed);
			#line 286 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        ";
			#line 286 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			if((content.posts).begin()!=(content.posts).end()) {
				#line 287 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"            ";
				#line 287 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				for(CPPCMS_TYPEOF((content.posts).begin()) p_ptr=(content.posts).begin(),p_ptr_end=(content.posts).end();p_ptr!=p_ptr_end;++p_ptr) {
				#line 287 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				CPPCMS_TYPEOF(*p_ptr) &p=*p_ptr;
					#line 288 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"                ";
					#line 288 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					post(p,0,0);
					#line 289 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
					out()<<"\n"
						"            ";
				#line 289 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				} // end of item
				#line 290 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
			#line 290 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			 } else {
				#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
				out()<<"\n"
					"        ";
			#line 291 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			} // end of empty
			#line 293 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 293 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			actions("Bottom");
			#line 295 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"        <hr />\n"
				"        ";
			#line 295 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			navbar();
			#line 298 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
			out()<<"\n"
				"    </body>\n"
				"</html>\n"
				"";
		#line 298 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
		} // end of template render
	#line 300 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
	}; // end of class thread
#line 301 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
} // end of namespace my_skin
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
namespace {
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
 cppcms::views::generator my_generator; 
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
 struct loader { 
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
  loader() { 
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.name("my_skin");
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::ban,Content::Ban>("ban",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::board,Content::Board>("board",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::board_image,Content::BoardImage>("board_image",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::board_video,Content::BoardVideo>("board_video",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::error,Content::Error>("error",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::home,Content::Home>("home",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::not_found,Content::NotFound>("not_found",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::rules,Content::Rules>("rules",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
   my_generator.add_view<my_skin::thread,Content::Thread>("thread",true);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
    cppcms::views::pool::instance().add(my_generator);
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
 }
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
 ~loader() {  cppcms::views::pool::instance().remove(my_generator); }
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
} a_loader;
#line 302 "/home/darkangel/dev/qt/projects/active/ololord/src/lib/template/thread.tmpl"
} // anon 
