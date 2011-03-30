
#include <boost/tokenizer.hpp>

using boost::escaped_list_error;

template <class Char,
    class Traits = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
class my_escaped_list_separator {

  private:
    typedef std::basic_string<Char,Traits> string_type;
    struct char_eq {
      Char e_;
      char_eq(Char e):e_(e) { }
      bool operator()(Char c) {
        return Traits::eq(e_,c);
      }
    };
    string_type  escape_;
    string_type  c_;
    string_type  quote_;
    string_type  comment_;
    bool last_;

    bool is_escape(Char e) {
      char_eq f(e);
      return std::find_if(escape_.begin(),escape_.end(),f)!=escape_.end();
    }
    bool is_c(Char e) {
      char_eq f(e);
      return std::find_if(c_.begin(),c_.end(),f)!=c_.end();
    }
    bool is_quote(Char e) {
      char_eq f(e);
      return std::find_if(quote_.begin(),quote_.end(),f)!=quote_.end();
    }
    bool is_commentStart(Char e) {
        char_eq f(e);
        return std::find_if(comment_.begin(), comment_.end(), f)!=comment_.end();
    }
    template <typename iterator, typename Token>
    void do_escape(iterator& next,iterator end,Token& tok) {
      if (++next == end)
        throw escaped_list_error(std::string("cannot end with escape"));
      if (Traits::eq(*next,'n')) {
        tok+='\n';
        return;
      }
      if (Traits::eq(*next,'r')) {
        tok+='\r';
        return;
      }
      else if (is_quote(*next)) {
        tok+=*next;
        return;
      }
      else if (is_c(*next)) {
        tok+=*next;
        return;
      }
      else if (is_escape(*next)) {
        tok+=*next;
        return;
      }
      else
        throw escaped_list_error(std::string("unknown escape sequence"));
    }

    public:

    explicit my_escaped_list_separator(Char  e = '\\',
                                    Char c = ',',Char  q = '\"', Char  s = ';')
      : escape_(1,e), c_(1,c), quote_(1,q), comment_(1,s), last_(false) { }

    my_escaped_list_separator(string_type e, string_type c, string_type q, string_type s)
      : escape_(e), c_(c), quote_(q), comment_(s), last_(false) { }

    void reset() {last_=false;}

    Char qt;

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      bool bInQuote = false;
      tok = Token();

      if (next == end) {
        if (last_) {
          last_ = false;
          return true;
        }
        else
          return false;
      }
      last_ = false;
      for (;next != end;++next) {
        if (is_escape(*next)) {
          do_escape(next,end,tok);
        }
        else if (is_c(*next) || is_commentStart(*next)) {
          if (!bInQuote) {
            // If we are not in quote, then we are done
            // The last character was a c, that means there is
            // 1 more blank field
            if (tok.size() > 0) {
                last_ = true; 
                if (is_commentStart(*next)) {
                    next = end;
                } else {
                    ++next;
                }
                return true;
            } else if (is_commentStart(*next)) {
                next = end;
                last_ = true;
                return false;
            }
          }
          else tok+=*next;
        }
        else if (is_quote(*next)) {
          if (!bInQuote) {
            qt = *next;
            bInQuote=!bInQuote;
          } else if (qt == *next) {
            bInQuote=!bInQuote;
          }
          tok += *next;
        }
        else {
          tok += *next;
        }
      }
      return true;
    }
  };
