Feature: Tag removal
  As someone using notes
  I can remove a tag
  In order to maintain their semantic

  Scenario: Removed tag disappear from the list
    Given I display the available pages
    When I remove a "tag" named "Optional"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Tags                              | folder              |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |
