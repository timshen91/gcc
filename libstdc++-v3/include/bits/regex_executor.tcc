// class template regex -*- C++ -*-

// Copyright (C) 2013-2015 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

/**
 *  @file bits/regex_executor.tcc
 *  This is an internal header file, included by other library headers.
 *  Do not attempt to use it directly. @headername{regex}
 */

namespace std _GLIBCXX_VISIBILITY(default)
{
namespace __detail
{
namespace __regex
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

  // Return whether now is at some word boundary.
  template<typename _Nfa_type, typename _Bi_iter>
    inline bool _Context<_Nfa_type, _Bi_iter>::
    _M_word_boundary() const
    {
      bool __left_is_word = false;
      if (_M_current != _M_begin
	  || (_M_match_flags & regex_constants::match_prev_avail))
	{
	  auto __prev = _M_current;
	  if (_M_is_word(*std::prev(__prev)))
	    __left_is_word = true;
	}
      bool __right_is_word =
        _M_current != _M_end && _M_is_word(*_M_current);

      if (__left_is_word == __right_is_word)
	return false;
      if (__left_is_word && !(_M_match_flags & regex_constants::match_not_eow))
	return true;
      if (__right_is_word && !(_M_match_flags & regex_constants::match_not_bow))
	return true;
      return false;
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
  template<_Search_mode __search_mode>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_match_impl(_StateIdT __start)
    {
      if (__search_mode == _Search_mode::_Match)
	return _M_this()->_M_search_from_first(__start);

      if (_M_this()->_M_search_from_first(__start))
	return true;
      if (_M_this()->_M_match_flags & regex_constants::match_continuous)
	return false;
      _M_this()->_M_match_flags |= regex_constants::match_prev_avail;
      while (_M_this()->_M_begin != _M_this()->_M_end)
	{
	  ++_M_this()->_M_begin;
	  if (_M_this()->_M_search_from_first(__start))
	    return true;
	}
      return false;
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_dfs(_StateIdT __state_id, _Submatch* __captures)
    {
      if (_M_this()->_M_handle_visit(__state_id, __captures))
	return false;

      const auto& __state = _M_this()->_M_nfa[__state_id];
      switch (__state._M_opcode())
	{
	  case _S_opcode_repeat:
	    return _M_this()->_M_handle_repeat(__state_id, __captures);
	  case _S_opcode_subexpr_begin:
	    return _M_this()->_M_handle_subexpr_begin(__state, __captures);
	  case _S_opcode_subexpr_end:
	    return _M_this()->_M_handle_subexpr_end(__state, __captures);
	  case _S_opcode_line_begin_assertion:
	    return _M_this()->_M_handle_line_begin_assertion(__state,
							     __captures);
	  case _S_opcode_line_end_assertion:
	    return _M_this()->_M_handle_line_end_assertion(__state, __captures);
	  case _S_opcode_word_boundary:
	    return _M_this()->_M_handle_word_boundary(__state, __captures);
	  case _S_opcode_subexpr_lookahead:
	    return _M_this()->_M_handle_subexpr_lookahead(__state, __captures);
	  case _S_opcode_repeated_match:
	    return _M_this()->_M_handle_repeated_match(__state, __captures);
	  case _S_opcode_match:
	    return _M_this()->_M_handle_match(__state_id, __captures);
	  case _S_opcode_backref:
	    return _M_this()->_M_handle_backref(__state, __captures);
	  case _S_opcode_accept:
	    return _M_this()->_M_handle_accept(__state, __captures);
	  case _S_opcode_alternative:
	    return _M_this()->_M_handle_alternative(__state, __captures);
	  case _S_opcode_unknown:
	  case _S_opcode_dummy:
	    __glibcxx_assert(false);
	}
      __glibcxx_assert(false);
      return false;
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_handle_line_begin_assertion(const _State_type& __state,
				   _Submatch* __captures)
    {
      return _M_this()->_M_at_begin() && this->_M_dfs(__state._M_next,
						      __captures);
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_handle_line_end_assertion(const _State_type& __state,
				 _Submatch* __captures)
    {
      return _M_this()->_M_at_end() && this->_M_dfs(__state._M_next,
						    __captures);
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_handle_word_boundary(const _State_type& __state, _Submatch* __captures)
    {
      return _M_this()->_M_word_boundary() == !__state._M_neg
	&& this->_M_dfs(__state._M_next, __captures);
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_handle_subexpr_lookahead(const _State_type& __state,
				_Submatch* __captures)
    {
      // Return whether now match the given sub-NFA.
      const auto __lookahead = [this](_StateIdT __next, _Submatch* __captures)
      {
	vector<_Submatch> __what(_M_this()->_M_sub_count());
	_Executor_type __sub(
	  _M_this()->_M_current, _M_this()->_M_end, _M_this()->_M_nfa,
	  _M_this()->_M_match_flags | regex_constants::match_continuous,
	  _Search_mode::_Search, __what.data());
	if (__sub.template _M_match_impl<_Search_mode::_Search>(__next))
	  {
	    for (size_t __i = 0; __i < __what.size(); __i++)
	      if (__what[__i].matched)
		__captures[__i] = __what[__i];
	    return true;
	  }
	return false;
      };

      // Here __state._M_alt offers a single start node for a sub-NFA.
      // We recursively invoke our algorithm to match the sub-NFA.
      return __lookahead(__state._M_alt, __captures) == !__state._M_neg
	&& this->_M_dfs(__state._M_next, __captures);
    }

  template<typename _Nfa_type, typename _Bi_iter, typename _Executor_type>
    bool _Executor_mixin<_Nfa_type, _Bi_iter, _Executor_type>::
    _M_handle_alternative(const _State_type& __state, _Submatch* __captures)
    {
      if (_M_this()->_M_nfa._M_options() & regex_constants::ECMAScript)
	{
	  return this->_M_dfs(__state._M_alt, __captures)
	    || this->_M_dfs(__state._M_next, __captures);
	}
      else
	{
	  // Try both and compare the result.
	  auto __ret1 = this->_M_dfs(__state._M_alt, __captures);
	  auto __ret2 = this->_M_dfs(__state._M_next, __captures);
	  return __ret1 || __ret2;
	}
    }

  template<typename _Bi_iter>
    bool
    __leftmost_longest(const sub_match<_Bi_iter>* __lhs,
		       const sub_match<_Bi_iter>* __rhs, size_t __size)
    {
      for (size_t __i = 0; __i < __size; __i++, __lhs++, __rhs++)
	{
	  if (!__lhs->matched)
	    return false;
	  if (!__rhs->matched)
	    return true;
	  if (__lhs->first < __rhs->first)
	    return true;
	  if (__lhs->first > __rhs->first)
	    return false;
	  if (__lhs->second > __rhs->second)
	    return true;
	  if (__lhs->second < __rhs->second)
	    return false;
	}
      return false;
    }

  // DFS mode:
  //
  // It applies a Depth-First-Search (aka backtracking) on given NFA and input
  // string.
  // At the very beginning the executor stands in the start state, then it
  // tries every possible state transition in current state recursively. Some
  // state transitions consume input string, say, a single-char-matcher or a
  // back-reference matcher; some don't, like assertion or other anchor nodes.
  // When the input is exhausted and/or the current state is an accepting
  // state, the whole executor returns true.
  //
  // TODO: This approach is exponentially slow for certain input.
  //       Try to compile the NFA to a DFA.
  //
  // Time complexity: \Omega(match_length), O(2^(_M_nfa.size()))
  // Space complexity: \theta(match_results.size() + match_length)
  //
  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_search_from_first(_StateIdT __start)
    {
      this->_M_current = this->_M_begin;
      return this->_M_dfs(__start, this->_M_current_results());
    }

  // ECMAScript 262 [21.2.2.5.1] Note 4:
  // ...once the minimum number of repetitions has been satisfied, any more
  // expansions of Atom that match the empty character sequence are not
  // considered for further repetitions.
  //
  // POSIX doesn't specify this, so let's keep them consistent.
  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_nonreentrant_repeat(_StateIdT __i, _StateIdT __alt,
			   _Submatch* __captures)
    {
      auto __back = _M_last_rep_visit;
      _M_last_rep_visit = make_pair(__i, this->_M_current);
      auto __ret = this->_M_dfs(__alt, __captures);
      _M_last_rep_visit = std::move(__back);
      return __ret;
    };

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_subexpr_begin(const _State_type& __state, _Submatch* __captures)
    {
      auto& __res = __captures[__state._M_subexpr];
      auto __back = __res.first;
      __res.first = this->_M_current;
      auto __ret = this->_M_dfs(__state._M_next, __captures);
      if (_M_is_ecma() && __ret)
	return true;
      __res.first = __back;
      return __ret;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_subexpr_end(const _State_type& __state, _Submatch* __captures)
    {
      auto& __res = __captures[__state._M_subexpr];
      auto __back = __res;
      __res.second = this->_M_current;
      __res.matched = true;
      auto __ret = this->_M_dfs(__state._M_next, __captures);
      if (_M_is_ecma() && __ret)
	return true;
      __res = __back;
      return __ret;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_repeat(_StateIdT __state_id, _Submatch* __captures)
    {
      // The most recent repeated state visit is the same, and this->_M_current
      // doesn't change since then. Shouldn't continue dead looping.
      if (_M_last_rep_visit.first == __state_id
	  && _M_last_rep_visit.second == this->_M_current)
	return false;
      const auto& __state = this->_M_nfa[__state_id];
      // Greedy.
      if (!__state._M_neg)
	{
	  if (this->_M_is_ecma())
	    {
	      return _M_nonreentrant_repeat(__state_id, __state._M_alt,
					    __captures)
		|| this->_M_dfs(__state._M_next, __captures);
	    }
	  else
	    {
	      auto __ret1 = _M_nonreentrant_repeat(__state_id, __state._M_alt,
						   __captures);
	      auto __ret2 = this->_M_dfs(__state._M_next, __captures);
	      return __ret1 || __ret2;
	    }
	}
      else // Non-greedy mode
	{
	  __glibcxx_assert(this->_M_is_ecma());
	  return this->_M_dfs(__state._M_next, __captures)
	    || _M_nonreentrant_repeat(__state_id, __state._M_alt, __captures);
	}
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_repeated_match(const _State_type& __state, _Submatch* __captures)
    {
      // TODO: Don't use std::function as _Matcher. Use virtual classes and
      // expose not only single match, but also multi-match virtual
      // functions to avoid repeated virtual dispatches.
      if (_M_is_ecma())
	{
	  if (!__state._M_neg)
	    {
	      auto __original = this->_M_current;
	      while (this->_M_current != this->_M_end
		     && __state._M_matches(*this->_M_current))
		++this->_M_current;
	      while (1)
		{
		  if (this->_M_dfs(__state._M_next, __captures))
		    {
		      this->_M_current = std::move(__original);
		      return true;
		    }
		  if (this->_M_current == __original)
		    {
		      this->_M_current = std::move(__original);
		      return false;
		    }
		  --this->_M_current;
		}
	      __glibcxx_assert(false);
	      return false;
	    }
	  else
	    {
	      if (this->_M_dfs(__state._M_next, __captures))
		return true;
	      auto __original = this->_M_current;
	      while (this->_M_current != this->_M_end
		     && __state._M_matches(*this->_M_current))
		{
		  ++this->_M_current;
		  if (this->_M_dfs(__state._M_next, __captures))
		    {
		      this->_M_current = std::move(__original);
		      return true;
		    }
		}
	      this->_M_current = std::move(__original);
	      return false;
	    }
	}
      else
	{
	  bool __ret = false;
	  auto __original = this->_M_current;
	  __ret |= this->_M_dfs(__state._M_next, __captures);
	  while (this->_M_current != this->_M_end
		 && __state._M_matches(*this->_M_current))
	    {
	      ++this->_M_current;
	      if (this->_M_dfs(__state._M_next, __captures))
		__ret |= this->_M_dfs(__state._M_next, __captures);
	    }
	  this->_M_current = std::move(__original);
	  return __ret;
	}
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_match(_StateIdT __state_id, _Submatch* __captures)
    {
      const auto& __state = this->_M_nfa[__state_id];
      if (this->_M_current == this->_M_end)
	return false;
      if (__state._M_matches(*this->_M_current))
	{
	  ++this->_M_current;
	  auto __ret = this->_M_dfs(__state._M_next, __captures);
	  --this->_M_current;
	  return __ret;
	}
      return false;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_backref(const _State_type& __state, _Submatch* __captures)
    {
      auto& __submatch = __captures[__state._M_backref_index];
      if (__submatch.matched)
	{
	  auto __last = this->_M_current;
	  for (auto __tmp = __submatch.first;
	       __last != this->_M_end && __tmp != __submatch.second;
	       ++__tmp)
	    ++__last;
	  if (this->_M_nfa._M_options() & regex_constants::collate)
	    {
	      const auto& __traits = this->_M_nfa._M_traits;
	      if (__traits.transform(__submatch.first, __submatch.second)
		  != __traits.transform(this->_M_current, __last))
		return false;
	    }
	  else
	    {
	      if (!std::equal(__submatch.first, __submatch.second,
			      this->_M_current))
		return false;
	    }
	  auto __back = this->_M_current;
	  this->_M_current = __last;
	  auto __ret = this->_M_dfs(__state._M_next, __captures);
	  this->_M_current = std::move(__back);
	  return __ret;
	}
      else
	{
	  // ECMAScript [21.2.2.9] Note:
	  // If the regular expression has n or more capturing parentheses
	  // but the nth one is undefined because it has not captured
	  // anything, then the backreference always succeeds.
	  return this->_M_dfs(__state._M_next, __captures);
	}
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Dfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_accept(const _State_type& __state, _Submatch* __captures)
    {
      bool __has_sol = false;
      if (this->_M_search_mode == _Search_mode::_Match)
	__has_sol = this->_M_current == this->_M_end;
      else
	__has_sol = true;
      if (this->_M_current == this->_M_begin
	  && (this->_M_match_flags & regex_constants::match_not_null))
	__has_sol = false;
      if (__has_sol)
	this->_M_update();
      return __has_sol;
    }

  template<typename _Bi_iter, typename _Traits>
    bool _Bfs_mixin<_Bfs_executor<_Bi_iter, _Traits, _Style::_Posix>>::
    _M_leftmost_longest(const _Submatch* __lhs, const _Submatch* __rhs,
			size_t __size)
    {
      for (size_t __i = 0; __i < __size; __i++, __lhs++, __rhs++)
	{
	  bool __lhs_half_matched = _M_half_matched(__lhs->first);
	  bool __rhs_half_matched = _M_half_matched(__rhs->first);
	  if (__lhs_half_matched && !__rhs_half_matched)
	    return true;
	  if (!__lhs_half_matched && __rhs_half_matched)
	    return false;
	  if (__lhs_half_matched && __rhs_half_matched)
	    {
	      if (__lhs->first < __rhs->first)
		return true;
	      if (__lhs->first > __rhs->first)
		return false;
	      if (__lhs->matched && __rhs->matched)
		{
		  if (__lhs->second > __rhs->second)
		    return true;
		  if (__lhs->second < __rhs->second)
		    return false;
		}
	    }
	}
      return false;
    }

  // ------------------------------------------------------------
  //
  // BFS mode:
  //
  // Russ Cox's article (http://swtch.com/~rsc/regexp/regexp1.html)
  // explained this algorithm clearly.
  //
  // It first computes epsilon closure (states that can be achieved without
  // consuming characters) for every state that's still matching,
  // using the same DFS algorithm, but doesn't re-enter states (using
  // _M_visited to check), nor follow _S_opcode_match.
  //
  // Then apply DFS using every _S_opcode_match (in _M_match_queue)
  // as the start state.
  //
  // It significantly reduces potential duplicate states, so has a better
  // upper bound; but it requires more overhead.
  //
  // Time complexity: \Omega(match_length * match_results.size())
  //                  O(match_length * _M_nfa.size() * match_results.size())
  // Space complexity: \Omega(_M_nfa.size() + match_results.size())
  //                   O(_M_nfa.size() * match_results.size())
  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Bfs_executor<_Nfa_type, _BiIter, __style>::
    _M_search_from_first(_StateIdT __start)
    {
      this->_M_current = this->_M_begin;
      bool __ret = false;
      vector<pair<_StateIdT, vector<_Submatch>>> __old_queue;
      __old_queue.emplace_back(__start,
			       vector<_Submatch>(this->_M_sub_count()));
      while (!__old_queue.empty())
	{
	  bool __has_sol = false;
	  this->_M_clear();
	  for (auto& __task : __old_queue)
	    if (this->_M_dfs(__task.first, __task.second.data()))
	      {
		__has_sol = true;
		if (this->_M_is_ecma())
		  break;
	      }
	  std::swap(__old_queue, _M_match_queue);
	  _M_match_queue.clear();
	  if (__has_sol)
	    __ret = true;
	  if (this->_M_current == this->_M_end)
	    break;
	  ++this->_M_current;
	}
      return __ret;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Bfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_subexpr_begin(const _State_type& __state, _Submatch* __captures)
    {
      auto& __res = __captures[__state._M_subexpr];
      auto __back = __res.first;
      __res.first = this->_M_current;
      auto __ret = this->_M_dfs(__state._M_next, __captures);
      __res.first = __back;
      return __ret;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Bfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_subexpr_end(const _State_type& __state, _Submatch* __captures)
    {
      auto& __res = __captures[__state._M_subexpr];
      auto __back = __res;
      __res.second = this->_M_current;
      __res.matched = true;
      auto __ret = this->_M_dfs(__state._M_next, __captures);
      __res = __back;
      return __ret;
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Bfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_repeat(_StateIdT __state_id, _Submatch* __captures)
    {
      const auto& __state = this->_M_nfa[__state_id];
      // Greedy.
      if (!__state._M_neg)
	{
	  if (this->_M_is_ecma())
	    {
	      return this->_M_dfs(__state._M_alt, __captures)
		|| this->_M_dfs(__state._M_next, __captures);
	    }
	  else
	    {
	      auto __ret1 = this->_M_dfs(__state._M_alt, __captures);
	      auto __ret2 = this->_M_dfs(__state._M_next, __captures);
	      return __ret1 || __ret2;
	    }
	}
      else // Non-greedy mode
	{
	  __glibcxx_assert(this->_M_is_ecma());
	  return this->_M_dfs(__state._M_next, __captures)
	    || this->_M_dfs(__state._M_alt, __captures);
	}
    }

  template<typename _Nfa_type, typename _BiIter, _Style __style>
    bool _Bfs_executor<_Nfa_type, _BiIter, __style>::
    _M_handle_accept(const _State_type& __state, _Submatch* __captures)
    {
      bool __has_sol = false;
      if (this->_M_search_mode == _Search_mode::_Match)
	__has_sol = this->_M_current == this->_M_end;
      else
	__has_sol = true;
      if (this->_M_current == this->_M_begin
	  && (this->_M_match_flags & regex_constants::match_not_null))
	__has_sol = false;
      if (__has_sol)
	{
	  if (this->_M_is_ecma()
	      || __leftmost_longest(__captures, _M_results,
				    this->_M_sub_count()))
	    {
	      std::copy(__captures, __captures + this->_M_sub_count(),
			_M_results);
	    }
	}
      return __has_sol;
    }

_GLIBCXX_END_NAMESPACE_VERSION
} // namespace __regex
} // namespace __detail
} // namespace
