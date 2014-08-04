﻿module Fplfs.RegexParser
    type RegexAstNode =
        | CharAstNode of char
        | SequenceAstNode of RegexAstNode list
        | ZeroOrMoreAstNode of RegexAstNode
        | OneOrMoreAstNode of RegexAstNode
        | ZeroOrOneAstNode of RegexAstNode

    let parseRegex (s : string) =
        match s.Length with
        | 0 -> failwith "String must not be empty"
        | _ ->
            let rec parseChars acc pos =
                let handleModifierChar c f = function
                    | [] -> failwith (sprintf "'%c' found with no preceding expression" c)
                    | head :: tail -> f head :: tail

                if pos >= s.Length then acc
                else
                    let acc' =
                        match s.[pos] with
                        | '*' -> handleModifierChar '*' ZeroOrMoreAstNode acc
                        | '+' -> handleModifierChar '+' OneOrMoreAstNode acc
                        | '?' -> handleModifierChar '?' ZeroOrOneAstNode acc
                        | x -> CharAstNode x :: acc

                    parseChars acc' (pos + 1)

            match parseChars [] 0 with
            | [x] -> x
            | x -> SequenceAstNode (List.rev x)
